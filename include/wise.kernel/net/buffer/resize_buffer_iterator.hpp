#pragma once 

#include <iterator>

namespace wise {
namespace kernel {

/// a contiguous, random iterator for resize_buffer
template <typename Elem>
class resize_buffer_iterator
	: public std::iterator<
	std::random_access_iterator_tag,
	Elem,
	ptrdiff_t,
	Elem*,
	Elem&>
{
public:
	resize_buffer_iterator(Elem* ptr = nullptr)
		: elem_(ptr)
	{}

	resize_buffer_iterator(const resize_buffer_iterator<Elem>& iter) = default;

	~resize_buffer_iterator() {}

	resize_buffer_iterator<Elem>& operator=(const resize_buffer_iterator<Elem>& iter) = default;

	resize_buffer_iterator<Elem>& operator=(Elem* ptr) { elem_ = ptr; return (*this); }

	operator bool() const
	{
		if (elem_)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool operator==(const resize_buffer_iterator<Elem>& iter) const
	{
		return (elem_ == iter.get_const_elem());
	}

	bool operator!=(const resize_buffer_iterator<Elem>& iter) const
	{
		return (elem_ != iter.get_const_elem());
	}

	resize_buffer_iterator<Elem>& operator+=(const ptrdiff_t& movement)
	{
		elem_ += movement;
		return (*this);
	}

	resize_buffer_iterator<Elem>& operator-=(const ptrdiff_t& movement)
	{
		elem_ -= movement;
		return (*this);
	}

	resize_buffer_iterator<Elem>& operator++()
	{
		++elem_;
		return (*this);
	}

	resize_buffer_iterator<Elem>& operator--()
	{
		--elem_;
		return (*this);
	}

	resize_buffer_iterator<Elem>  operator+(const ptrdiff_t& movement)
	{
		auto old_ptr = elem_;
		elem_ += movement;
		auto temp(*this);
		elem_ = old_ptr;
		return temp;
	}

	resize_buffer_iterator<Elem>  operator-(const ptrdiff_t& movement)
	{
		auto old_ptr = elem_;
		elem_ -= movement;
		auto temp(*this);
		elem_ = old_ptr;
		return temp;
	}

	ptrdiff_t operator-(const resize_buffer_iterator<Elem>& iter)
	{
		return std::distance(iter.get_elem(), this->get_elem());
	}

	Elem& operator*() { return *elem_; }

	const Elem& operator*()const { return *elem_; }

	Elem* operator->() { return elem_; }


private:
	Elem* get_elem()const { return elem_; }

	const Elem* get_const_elem()const { return elem_; }

private:
	Elem* elem_ = nullptr;
};

} // kernel
} // wise
