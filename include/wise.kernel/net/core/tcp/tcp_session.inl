#pragma once 

#include <wise/net/session.hpp>

namespace wise {

inline
const session::id& session::get_id() const
{
	return id_;
}

inline
session::id::id(uint32_t value)
	: value_(value)
{
}

inline
session::id::id(uint16_t seq, uint16_t index)
{
	full_.seq_ = seq; 
	full_.index_ = index;
	WISE_ASSERT(is_valid());
}

inline
bool session::id::is_valid() const
{
	return full_.seq_ > 0 && full_.index_ >= 0;
}

inline
const uint32_t session::id::get_value() const
{
	return value_;
}

inline
const uint16_t session::id::get_seq() const
{
	return full_.seq_;
}

inline
const uint16_t session::id::get_index() const
{
	return full_.index_;
}

inline
bool session::id::operator==(const id& rhs) const
{
	return value_ == rhs.value_;
}

inline
bool session::id::operator!=(const id& rhs) const
{
	return value_ != rhs.value_;
}

inline
bool session::id::operator < (const id& rhs) const
{
	return value_ < rhs.value_;
}


} // wise


