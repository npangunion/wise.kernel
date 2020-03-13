#pragma once 

#include <wise.kernel/core/macros.hpp>
#include <spdlog/fmt/fmt.h>
#include <memory>

namespace wise {
namespace kernel {

/// 메세지 전파에 사용하는 토픽 
/**
 * category | group | type 으로 나누어서 처리하는 것을 명시적으로 요구.
 */
class topic
{
public:
	using key_t = uint32_t;
	using category_t = uint8_t;
	using group_t = uint8_t;
	using type_t = uint16_t;

public:
	topic()
		: key_(0)
	{
	}

	explicit topic(key_t key)
		: key_(key)
	{
	}

	explicit topic(category_t cat, group_t group, type_t type)
		: key_(cat << 24 | group << 16 | type)
	{
	}

	topic(const topic& rhs)
	{
		*this = rhs;
	}

	topic& operator = (const topic& rhs)
	{
		key_ = rhs.key_;

		return *this;
	}

	key_t get_key() const
	{
		return key_;
	}

	category_t get_category() const
	{
		return (key_ >> 24) & 0x000000FF;
	}

	group_t get_group() const
	{
		return (key_ >> 16) & 0x000000FF;
	}

	type_t get_type() const
	{
		return key_ & 0x0000FFFF;
	}

	topic get_group_topic() const
	{
		return topic(get_category(), get_group(), 0);
	}

	bool is_valid() const
	{
		return key_ > 0;
	}

	bool match(const topic& other)
	{
		if (get_category() == other.get_category() && get_group() == other.get_group())
		{
			if (get_type() == 0)
			{
				return true;
			}

			return get_type() == other.get_type();
		}

		return false;
	}

	topic& operator=(key_t key)
	{
		key_ = key;

		return *this;
	}

	bool operator == (const topic& rhs) const
	{
		return key_ == rhs.key_;
	}

	bool operator != (const topic& rhs) const
	{
		return !(operator == (rhs));
	}

	bool operator < (const topic& rhs) const
	{
		return key_ < rhs.key_;
	}

	bool operator > (const topic& rhs) const
	{
		return key_ > rhs.key_;
	}

	std::string get_desc() const
	{
		return topic::get_desc(*this);
	}

	static std::string get_desc(const topic& pic)
	{
		return fmt::format("[{}/{}/{}]",
			pic.get_category(), pic.get_group(), pic.get_type());
	}

private:
	key_t key_;
};

} // kernel
} // wise

#include <unordered_map>

namespace std {

template <>
struct hash<::wise::kernel::topic>
{
	std::size_t operator()(const ::wise::kernel::topic& k) const
	{
		auto key = static_cast<uint32_t>(k.get_key());

		return std::hash<uint32_t>()(key);
	}
};

} // std
