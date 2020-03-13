#pragma once 

#include "config.hpp"
#include "spinlock.hpp"
#include "tick.hpp"
#include "concurrent_queue.hpp"
#include "detail/malloc_allocator.hpp"
#include <map>

namespace wise {
namespace kernel {

class mem_writer
{
public:
	virtual void write(const char* stat) = 0;
};

/// memory allocation tracker 
/**
 * This is a simple and practical tracker,
 * using wise_new / wise_shared / wise_unqiue
 * and a global new / delete operator.
 * @see test_mem_tracker.cpp
 */
class mem_tracker
{
public:
	static mem_tracker& inst();

private:
	mem_tracker();

public:
	~mem_tracker();

	/// writer의 소유권이 옮겨옴. 힙 할당.
	void set_writer(mem_writer* writer);

	/// enable tracking memory
	void enable();

	/// disable
	void disable();

	/// report current memory usage
	void report();

	/// internal use.
	void on_alloc(uintptr_t mem, std::size_t size, const char* cls);

	/// internal use.
	void on_dealloc(uintptr_t mem);

private:
	using mstring = std::basic_string<char, std::char_traits<char>, malloc_allocator<char>>;

	struct alloc
	{
		std::size_t size = 0;
		uint32_t	crc = 0;

		alloc(std::size_t sz, uint32_t _crc)
			: size(sz)
			, crc(_crc)
		{}
	};

	struct size_stat
	{
		std::size_t alloc_count = 0;
		std::size_t dealloc_count = 0;
		std::size_t size = 0;
	};

	struct class_stat : public size_stat
	{
		mstring cls;
	};

	struct raw_op
	{
		bool		alloc = true;
		uintptr_t	mem;
		std::size_t size;
		mstring		cls;
	};

	using lock_type = spinlock;
	using mmap = std::map<uintptr_t, alloc>;
	using smap = std::map<std::size_t, size_stat>;
	using cmap = std::map<uint32_t, class_stat>;
	using op_q = concurrent_queue<raw_op, malloc_allocator<raw_op>>;

private:
	void on_alloc_locked(uintptr_t mem, std::size_t size, const mstring& cls);

	void on_dealloc_locked(uintptr_t mem);

private:
	lock_type			mutex_;
	mem_writer* writer_ = nullptr;
	std::atomic<bool>	enable_ = true;

	op_q				ops_;				// 할당 / 해제 순서를 지키기 위해 큐 하나로 처리

	mmap				allocs_;			// 포인터별 할당 정보. 해제될 때 지움
	smap				size_stats_;		// 사이즈별 할당 정보.  
	cmap				class_stats_;		// file / line을 합친 crc 별 메모리 통계

	std::size_t			total_alloc_ = 0;
	std::size_t			total_dealloc_ = 0;
};

} // kernel
} // wise


template <typename Ty, typename... Args>
inline
Ty* wise_new(Args&&... args)
{
	Ty* p = new Ty(std::forward<Args>(args)...);

#if WISE_TRACK_MEMORY
	wise::kernel::mem_tracker::inst().on_alloc(
		reinterpret_cast<uintptr_t>(p), sizeof(Ty), typeid(Ty).name()
	);
#endif

	return p;
}

template <typename Ty, typename... Args>
inline
std::shared_ptr<Ty> wise_shared(Args&&... args)
{
	auto p = std::make_shared<Ty>(std::forward<Args>(args)...);

#if WISE_TRACK_MEMORY
	Ty* addr = p.get();

	// XXX: 32비트 빌드에서 확인. 윈도우 플래폼에서만 지원하도록 설정

	const int hack_shared_ptr_addr = 0x10;

	wise::kernel::mem_tracker::inst().on_alloc(
		reinterpret_cast<uintptr_t>(addr) - hack_shared_ptr_addr,
		sizeof(Ty), typeid(Ty).name()
	);
#endif
	return p;
}


template <typename Ty, typename... Args>
inline
std::unique_ptr<Ty> wise_unique(Args&&... args)
{
	auto p = std::make_unique<Ty>(std::forward<Args>(args)...);

#if WISE_TRACK_MEMORY
	Ty* addr = p.get();

	wise::kernel::mem_tracker::inst().on_alloc(
		reinterpret_cast<uintptr_t>(addr),
		sizeof(Ty), typeid(Ty).name()
	);
#endif

	return p;
}

