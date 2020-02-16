#include "stdafx.h"
#include "memory.hpp"

#include "exception.hpp"
#include "logger.hpp"
#include "util.hpp"
#include <iomanip>

#if WISE_TRACK_MEMORY 

void* operator new (std::size_t n) 
{
	void* p = std::malloc(n);
	return p;
}

void operator delete(void* p)
{
	wise::mem_tracker::inst().on_dealloc(reinterpret_cast<uintptr_t>(p));
	std::free(p);
}

#endif

namespace wise
{

class log_writer : public mem_writer
{
public: 
	void write(const char* stat) override
	{
		WISE_INFO("{}", stat);
	}
};

mem_tracker& mem_tracker::inst()
{
	static mem_tracker inst_;

	return inst_;
}

mem_tracker::mem_tracker()
{
	set_writer(new log_writer());
}

mem_tracker::~mem_tracker()
{
	// 소멸 중 dealloc 호출에서 메모리 에러 발생하여 disable 시킴 

	disable();

	delete writer_;
}

void mem_tracker::set_writer(mem_writer* writer)
{
	WISE_ASSERT(writer);

	if (writer_)
	{
		delete writer_;
	}

	writer_ = writer;
}

void mem_tracker::enable()
{
	enable_ = true;
}

void mem_tracker::disable()
{
	enable_ = false;
}

void mem_tracker::on_alloc(uintptr_t mem, std::size_t size, const char* cls)
{
	WISE_RETURN_IF(!enable_);

	// push
	{
		raw_op op;

		op.alloc = true;
		op.mem = mem;
		op.cls = cls;
		op.size = size;

		ops_.push(op);
	}

	if (!mutex_.try_lock())
	{
		return;
	}

	raw_op op;

	while (ops_.pop(op))
	{
		if (op.alloc)
		{
			on_alloc_locked(op.mem, op.size, op.cls);
		}
		else
		{
			on_dealloc_locked(op.mem);
		}
	}

	mutex_.unlock();
}

void mem_tracker::on_dealloc(uintptr_t mem)
{
	WISE_RETURN_IF(!enable_);

	// push
	{
		raw_op op;

		op.alloc = false;
		op.mem = mem;

		ops_.push(op);
	}

	if (!mutex_.try_lock())
	{
		return;
	}

	raw_op op;

	while (ops_.pop(op))
	{
		if (op.alloc)
		{
			on_alloc_locked(op.mem, op.size, op.cls);
		}
		else
		{
			on_dealloc_locked(op.mem);
		}
	}

	mutex_.unlock();
}

void mem_tracker::on_alloc_locked(
	uintptr_t mem, std::size_t size, const mstring& cls
)
{
	auto crc = get_crc32(cls.c_str(), cls.length());

	// alloc
	{
		auto iter = allocs_.find(mem);

		if (iter != allocs_.end())
		{
			WISE_ERROR(
				"allocation of unfreed memory is not allowed. "
				"class: {}, mem: 0x{:x}, size: {}", 
				cls.c_str(), mem, size
			);

			return;
		}

		allocs_.insert(mmap::value_type(mem, alloc{ size, crc }));

		total_alloc_ += size;
	}

	bool size_added = false;

	// size stat
	{
		auto iter = size_stats_.find(size);

		if (iter == size_stats_.end())
		{
			size_stat ss;
			ss.size = size;
			ss.alloc_count++;

			size_stats_.insert(smap::value_type(size, ss));

			size_added = true;
		}
		else
		{
			auto& ss = iter->second;
			ss.alloc_count++;
		}
	}

	bool cls_added = false;

	// class stat
	{
		auto iter = class_stats_.find(crc);

		if (iter == class_stats_.end())
		{
			class_stat cs;
			cs.cls = cls;
			cs.size = size;
			cs.alloc_count++;

			class_stats_.insert(cmap::value_type(crc, cs));

			cls_added = true;
		}
		else
		{
			auto& cs = iter->second;
			cs.alloc_count++;
		}
	}
}

void mem_tracker::on_dealloc_locked(uintptr_t mem)
{
	auto iter = allocs_.find(mem);
	WISE_RETURN_IF(iter == allocs_.end());

	auto& ae = iter->second;

	bool size_deleted = false;

	// size stat
	{
		auto siter = size_stats_.find(ae.size);
		WISE_ASSERT(siter != size_stats_.end());

		auto& ss = siter->second;
		ss.dealloc_count++;

		if (ss.alloc_count == ss.dealloc_count)
		{
			size_stats_.erase(siter);

			size_deleted = true;
		}
	}

	bool cls_deleted = false;

	// class stat
	{
		auto citer = class_stats_.find(ae.crc);
		WISE_ASSERT(citer != class_stats_.end());

		auto& cs = citer->second;
		cs.dealloc_count++;

		if (cs.dealloc_count == cs.alloc_count)
		{
			class_stats_.erase(citer);
			cls_deleted = true;
		}
	}

	total_dealloc_ += iter->second.size;

	// size는 다른 클래스가 같은 크기일 수도 있으므로 같이 지워지지는 않는다. 
	// 할당 블럭 정보는 해제될 때 바로 지운다. 
	allocs_.erase(iter);
}

void mem_tracker::report()
{
	std::ostringstream oss;

	oss << "\n--------------------------------------------------------------------\n";
	oss << "\tmemory report\n";
	oss << "--------------------------------------------------------------------\n";

	std::size_t total_remain_bytes = 0;

	// locked
	{
		std::lock_guard<lock_type> lock(mutex_);

		// size stat
		oss << "size stats:\n";

		for (auto& ss : size_stats_)
		{
			WISE_ASSERT(ss.second.alloc_count > ss.second.dealloc_count);

			auto diff = ss.second.alloc_count - ss.second.dealloc_count;

			oss << "\tsize: " << std::setw(12) << ss.second.size << "\tcount: " << std::setw(12) << diff << "\n";

			total_remain_bytes += ss.second.size * diff;
		}

		// class stat
		oss << "type stats:\n";

		for (auto& cs : class_stats_)
		{
			oss	<< "\tsize: "	<< std::setw(12) << cs.second.size 
				<< "\tcount: "	<< std::setw(12) << cs.second.alloc_count - cs.second.dealloc_count
				<< "\ttype: "	<< cs.second.cls;
			oss << "\n";
		}
	}

	oss << "\n";

	oss << "current      : " << std::setw(16) << total_remain_bytes / 1024 << " KB\n";
	oss << "total alloc  : " << std::setw(16) << total_alloc_ / 1024 << " KB\n";
	oss << "total dealloc: " << std::setw(16) << total_dealloc_ / 1024 << " KB\n";

	writer_->write(oss.str().c_str());
}

} // wise