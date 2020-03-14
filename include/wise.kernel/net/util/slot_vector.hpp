#pragma once

#include <wise.kernel/core/exception.hpp>
#include <deque>
#include <memory>
#include <vector>

namespace wise {
namespace kernel {

template <typename T>
class slot_vector
{
public:
	using ptr = std::shared_ptr<T>;

private:
	struct slot
	{
		ptr			sp_;
		uint16_t	seq_ = 0;

		slot(ptr sp, uint16_t seq)
			: sp_(sp)
			, seq_(seq)
		{
		}
	};

	union slot_id
	{
		using full_t = uint32_t;
		using seq_t = uint16_t;
		using index_t = uint16_t;

		full_t value_ = 0;

		struct full
		{
			seq_t seq_;
			index_t index_;		// index in slot vector
		} full_;

		explicit slot_id(full_t value = 0);
		slot_id(seq_t seq, index_t index);

		/// seq_ > 0 
		bool is_valid() const;

		const full_t get_value() const;
		const seq_t get_seq() const;
		const index_t get_index() const;

		bool operator==(const slot_id& rhs) const;
		bool operator!=(const slot_id& rhs) const;
		bool operator < (const slot_id& rhs) const;
	};

public:
	using id_t = typename slot_id::full_t;

public:
	slot_vector();

	~slot_vector();

	id_t add(ptr sp);

	ptr get(id_t id) const;

	bool del(id_t id);

	void clear();

	std::size_t each(std::function<void (ptr)> fn);

private:
	uint16_t get_free_slot_index();

private:
	std::vector<slot>		slots_;
	std::deque<uint16_t>    free_slots_;
	std::size_t				slot_count_;
};

template <typename T>
slot_vector<T>::slot_vector()
	: slots_()
	, free_slots_()
	, slot_count_(0)
{
}

template <typename T>
slot_vector<T>::~slot_vector()
{
}

template <typename T>
typename slot_vector<T>::id_t slot_vector<T>::add(ptr sp)
{
	uint16_t slot_idx = get_free_slot_index();
	uint16_t seq = 0;

	seq = slots_[slot_idx].seq_++;
	seq = (seq == 0 ? slots_[slot_idx].seq_++ : seq);

	WISE_ASSERT(seq > 0);

	slots_[slot_idx].sp_ = sp;
	slot_count_++;

	WISE_ENSURE(slots_[slot_idx].sp_);
	WISE_ENSURE(slot_count_ > 0);

	return slot_id(seq, slot_idx).get_value();
}

template <typename T>
typename slot_vector<T>::ptr slot_vector<T>::get(id_t id) const
{
	slot_id sid(id);

	return slots_[sid.get_index()].sp_;
}

template <typename T>
bool slot_vector<T>::del(id_t id)
{
	WISE_ASSERT(slot_count_ > 0);

	slot_id sid(id);

	slots_[sid.get_index()].sp_.reset();
	slot_count_--;

	free_slots_.push_back(sid.get_index());

	WISE_ASSERT(slot_count_ >= 0);
}

template <typename T>
void slot_vector<T>::clear()
{
	slots_.clear();
}

template <typename T>
std::size_t slot_vector<T>::each(std::function<void(ptr)> fn)
{
	std::size_t cnt = 0;

	for (auto& slot : slots_)
	{
		if (!!slot.sp_)
		{
			fn(slot.sp_);
			++cnt;
		}
	}

	return cnt;
}

template <typename T>
uint16_t slot_vector<T>::get_free_slot_index()
{
	if (slots_.size() >= UINT16_MAX)
	{
		WISE_THROW("network_impl slot limit reached");
	}

	if (free_slots_.empty())
	{
		slots_.push_back({ ptr(), 0 });

		return static_cast<uint16_t>(slots_.size() - 1);
	}

	auto index = free_slots_.front();

	free_slots_.pop_front();

	WISE_ASSERT(index < slots_.size());

	return index;
}

template <typename T>
slot_vector<T>::slot_id::slot_id(uint32_t value)
	: value_(value)
{
}

template <typename T>
slot_vector<T>::slot_id::slot_id(uint16_t seq, uint16_t index)
{
	full_.seq_ = seq;
	full_.index_ = index;
	WISE_ASSERT(is_valid());
}

template <typename T>
const uint32_t slot_vector<T>::slot_id::get_value() const
{
	return value_;
}

template <typename T>
const uint16_t slot_vector<T>::slot_id::get_seq() const
{
	return full_.seq_;
}

template <typename T>
const uint16_t slot_vector<T>::slot_id::get_index() const
{
	return full_.index_;
}

template <typename T>
bool slot_vector<T>::slot_id::operator==(const slot_id& rhs) const
{
	return value_ == rhs.value_;
}

template <typename T>
bool slot_vector<T>::slot_id::operator!=(const slot_id& rhs) const
{
	return value_ != rhs.value_;
}

template <typename T>
bool slot_vector<T>::slot_id::operator < (const slot_id& rhs) const
{
	return value_ < rhs.value_;
}

} // kernel 
} // wise
