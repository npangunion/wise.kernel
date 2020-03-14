#pragma once

#include <wise/net/detail/buffer/resize_buffer.hpp>
#include <wise/base/logger.hpp>
#include <wise/base/date.hpp>

namespace wise
{

class zen_packer;

struct ipackable
{
	virtual bool pack(zen_packer& packer) const = 0;
	virtual bool unpack(zen_packer& packer)= 0;
};

#define DEFINE_PACK(Type) \
template <> bool pack(const Type& s) \
{ \
	static_assert( \
		std::is_fundamental<Type>::value || std::is_enum<Type>::value, \
		"pack> Type must be fundamental" \
		); \
	WISE_RETURN_IF(!is_valid_, false); \
	if (is_le_) \
	{ \
		buf_.append((uint8_t*)&s, sizeof(Type)); \
	} \
	else \
	{ \
		Type swapped = swap_endian(s); \
		buf_.append((uint8_t*)&swapped, sizeof(Type)); \
	} \
	return is_valid_; \
}

#define DEFINE_UNPACK(Type) \
template <> bool unpack(Type& s) \
{ \
	static_assert( \
		std::is_fundamental<Type>::value || std::is_enum<Type>::value, \
		"unpack> Type must be fundamental" \
		); \
	WISE_RETURN_IF(!is_valid_, false); \
	if (is_le_) \
	{ \
		is_valid_ = buf_.read((uint8_t*)&s, sizeof(Type)); \
	} \
	else \
	{ \
		Type o = Type(); \
		is_valid_ = buf_.read((uint8_t*)&o, sizeof(Type)); \
		if (is_valid_) \
		{ \
			s = swap_endian(o); \
		} \
	} \
	return is_valid_; \
}

template <typename T> 
bool pack(zen_packer& packer, const T& tv)
{
	return tv.pack(packer);
}

template <typename T> 
bool unpack(zen_packer& packer, T& tv)
{
	return tv.unpack(packer);
}

// serialize / deserialize to / from resize_buffer
class zen_packer
{
public:
	struct config 
	{
		static const std::size_t max_container_size = 8 * 1024;
		static const std::size_t max_string_size = 8 * 1024;
	};

	zen_packer(resize_buffer& buf)
		: buf_(buf)
	{
		// 리틀 엔디언을 기본 (인텔 기준)으로 하고 빅 엔디언일 경우 swap한다.
		int num = 1;
		is_le_ = (*(char*)&num == 1); // 
	}

	/// 바이트 배열
	bool pack(uint8_t* buf, std::size_t len);
	bool unpack(uint8_t* buf, std::size_t len);

	/// ipackable. 
	template <class T> bool pack(const T& s);
	template <class T> bool unpack(T& s);

	/// enum 타잎
	template <class T> bool pack_enum(const T& s);
	template <class T> bool unpack_enum(T& s);

	/// string
	template<> bool pack(const std::string& s);
	template<> bool unpack(std::string& s);

	template<> bool pack(const std::wstring& s);
	template<> bool unpack(std::wstring& s);

	/// date
	template<> bool pack(const date& s);
	template<> bool unpack(date& s);

	/// timestamp
	template<> bool pack(const timestamp& s);
	template<> bool unpack(timestamp& s);

	/// bool
	template<> bool pack(const bool& s);
	template<> bool unpack(bool& s);

	/// vector
	template <class T> bool pack(const std::vector<T>& v);
	template <class T> bool unpack(std::vector<T>& v);

	/// map
	template <class T1, class T2> bool pack(const std::map<T1, T2>& v);
	template <class T1, class T2> bool unpack(std::map<T1, T2>& v);

	/// 정수, 실수. 특수화로 처리
	DEFINE_PACK(uint8_t)
	DEFINE_PACK(uint16_t)
	DEFINE_PACK(uint32_t)
	DEFINE_PACK(uint64_t)
	DEFINE_PACK(int8_t)
	DEFINE_PACK(int16_t)
	DEFINE_PACK(int32_t)
	DEFINE_PACK(int64_t)
	DEFINE_PACK(float)
	DEFINE_PACK(double)

	DEFINE_UNPACK(uint8_t)
	DEFINE_UNPACK(uint16_t)
	DEFINE_UNPACK(uint32_t)
	DEFINE_UNPACK(uint64_t)
	DEFINE_UNPACK(int8_t)
	DEFINE_UNPACK(int16_t)
	DEFINE_UNPACK(int32_t)
	DEFINE_UNPACK(int64_t)
	DEFINE_UNPACK(float)
	DEFINE_UNPACK(double)

	bool is_valid() const
	{
		return is_valid_;
	}

private: 
	template <typename T> T swap_endian(const T& u);

	bool convert(const std::wstring& src, std::vector<uint8_t>& out);
	
	bool convert(const std::vector<uint8_t>& src, std::wstring& out);

private: 
	resize_buffer& buf_;
	bool is_le_ = true;
	bool is_valid_ = true;
};


inline bool zen_packer::pack(uint8_t* buf, std::size_t len)
{
	WISE_RETURN_IF(!is_valid_, false); 

	buf_.append(buf, len);

	return is_valid_;
}

inline bool zen_packer::unpack(uint8_t* buf, std::size_t len)
{
	WISE_RETURN_IF(!is_valid_, false);

	is_valid_ = buf_.read(buf, len);

	return is_valid_;
}

template <class T>
inline bool zen_packer::pack_enum(const T& s)
{
	static_assert(
		std::is_fundamental<T>::value || std::is_enum<T>::value,
		"pack> Type must be fundamental"
		);

	WISE_RETURN_IF(!is_valid_, false);

	int32_t val = static_cast<int32_t>(s); 
	is_valid_ = pack(val);

	return is_valid_;
}

template <class T>
inline bool zen_packer::unpack_enum(T& s)
{
	static_assert(
		std::is_fundamental<T>::value || std::is_enum<T>::value,
		"unpack> Type must be fundamental"
		);

	WISE_RETURN_IF(!is_valid_, false);

	int32_t val = 0;

	is_valid_ = unpack(val);
	WISE_RETURN_IF(!is_valid_, false);

	s = static_cast<T>(val);

	return is_valid_;
}

template<class T> 
inline bool zen_packer::pack(const T& s)
{
	WISE_RETURN_IF(!is_valid_, false);

	return ::wise::pack(*this, s);
}

template<class T> 
inline bool zen_packer::unpack(T& s)
{
	WISE_RETURN_IF(!is_valid_, false);

	return ::wise::unpack(*this, s);
}

template<>
inline bool zen_packer::pack(const std::string& s)
{
	WISE_RETURN_IF(!is_valid_, false);

	WISE_ASSERT(config::max_string_size >= s.length());

	if (config::max_container_size < s.length())
	{
		WISE_ERROR(
			"pack string - size: {} over than maxSize : {}",
			s.length(), config::max_string_size
		);

		is_valid_ = false;

		return is_valid_;
	}

	uint16_t len = static_cast<uint16_t>(s.length());

	is_valid_ = pack(len);
	WISE_RETURN_IF(!is_valid_, false);

	if (len > 0)
	{
		is_valid_ = pack((uint8_t*)s.c_str(), len);
		WISE_RETURN_IF(!is_valid_, false);
	}

	WISE_ENSURE(is_valid_);

	return is_valid_;
}


template<>
inline bool zen_packer::unpack(std::string& s)
{
	WISE_RETURN_IF(!is_valid_, false);

	uint16_t len = 0;

	is_valid_ = unpack(len);

	WISE_ASSERT(config::max_string_size >= len);

	if (config::max_container_size < len)
	{
		WISE_ERROR(
			"unpack string - size: {} over than maxSize : {}",
			len, config::max_string_size
		);

		is_valid_ = false;

		return is_valid_;
	}

	if (len > 0)
	{
		char tv[config::max_string_size + 1] = { 0, };

		is_valid_ = unpack((uint8_t*)tv, len);
		WISE_RETURN_IF(!is_valid_, false);

		tv[len] = '\0';

		s = tv;

		WISE_ENSURE(s.length() == len);
	}

	return is_valid_;
}

template<>
inline bool zen_packer::pack(const std::wstring& s)
{
	WISE_RETURN_IF(!is_valid_, false);

	// wstring을 플래폼에 맞게 utf8 문자열로 변환, 읽을 때는 utf8에서 플래폼에 맞는 인코딩으로 다시 변환

	if (s.length() > config::max_string_size)
	{
		WISE_ERROR(
			"pack wstring> size: {} over than maxSize: {}",
			s.length(), config::max_string_size
		);

		is_valid_ = false;
		return is_valid_;
	}

	std::vector<unsigned char> u8s;

	is_valid_ = convert(s, u8s);
	WISE_RETURN_IF(!is_valid_, false);

	return pack(u8s);
}

template<>
inline bool zen_packer::unpack(std::wstring& s)
{
	WISE_RETURN_IF(!is_valid_, false);

	std::vector<unsigned char> u8s;

	is_valid_ = unpack(u8s);
	WISE_RETURN_IF(!is_valid_, false);

	is_valid_ = convert(u8s, s);
	WISE_RETURN_IF(!is_valid_, false);

	if (s.length() > config::max_string_size)
	{
		WISE_ERROR(
			"unpack wstring> size: {} over than maxSize: {}",
			s.length(), config::max_string_size
		);

		is_valid_ = false;
		return is_valid_;
	}

	return is_valid_;
}

template<> 
inline bool zen_packer::pack(const date& s)
{
	WISE_RETURN_IF(!is_valid_, false); 

	WISE_RETURN_IF(!pack(s.year), false);
	WISE_RETURN_IF(!pack(s.month), false);
	WISE_RETURN_IF(!pack(s.day), false);

	return is_valid_;
}

template<> 
inline bool zen_packer::unpack(date& s)
{
	WISE_RETURN_IF(!is_valid_, false); 

	WISE_RETURN_IF(!unpack(s.year), false);
	WISE_RETURN_IF(!unpack(s.month), false);
	WISE_RETURN_IF(!unpack(s.day), false);

	return is_valid_;
}

template<> 
inline bool zen_packer::pack(const timestamp& s)
{
	WISE_RETURN_IF(!is_valid_, false); 

	WISE_RETURN_IF(!pack(s.year), false);
	WISE_RETURN_IF(!pack(s.month), false);
	WISE_RETURN_IF(!pack(s.day), false);
	WISE_RETURN_IF(!pack(s.hour), false);
	WISE_RETURN_IF(!pack(s.min), false);
	WISE_RETURN_IF(!pack(s.sec), false);
	WISE_RETURN_IF(!pack(s.fract), false);

	return is_valid_;
}

template<> 
inline bool zen_packer::unpack(timestamp& s)
{
	WISE_RETURN_IF(!is_valid_, false); 

	WISE_RETURN_IF(!unpack(s.year), false);
	WISE_RETURN_IF(!unpack(s.month), false);
	WISE_RETURN_IF(!unpack(s.day), false);
	WISE_RETURN_IF(!unpack(s.hour), false);
	WISE_RETURN_IF(!unpack(s.min), false);
	WISE_RETURN_IF(!unpack(s.sec), false);
	WISE_RETURN_IF(!unpack(s.fract), false);

	return is_valid_;
}

template<> 
inline bool zen_packer::pack(const bool& s)
{
	WISE_RETURN_IF(!is_valid_, false);

	uint8_t val = s == true ? 1 : 0;
	is_valid_ = pack(val);

	return is_valid_;
}


template<>
inline bool zen_packer::unpack(bool& s)
{
	WISE_RETURN_IF(!is_valid_, false);

	uint8_t val = 0;

	is_valid_ = unpack(val);
	WISE_RETURN_IF(!is_valid_, false);

	s = (val > 0);

	return is_valid_;
}

template <class T>
inline bool zen_packer::pack(const std::vector<T>& vec)
{
	WISE_RETURN_IF(!is_valid_, false);

	uint16_t	len = 0;

	WISE_ASSERT(config::max_container_size >= vec.size());

	if (config::max_container_size < vec.size())
	{
		WISE_ERROR(
			"pack vector<T> - size: {} over than maxSize : {}",
			vec.size(), config::max_container_size
		);

		is_valid_ = false;
		return is_valid_;
	}

	len = static_cast<uint16_t>(vec.size());

	is_valid_ = pack(len);

	WISE_RETURN_IF(!is_valid_, false);

	for (uint16_t i = 0; i < len; ++i)
	{
		is_valid_ = pack(vec[i]);

		WISE_RETURN_IF(!is_valid_, false);
	}

	WISE_ASSERT(is_valid_);

	return is_valid_;
}

template <class T>
inline bool zen_packer::unpack(std::vector<T>& vec)
{
	WISE_RETURN_IF(!is_valid_, false);

	uint16_t	len = 0; 

	is_valid_ = unpack(len);

	WISE_RETURN_IF(!is_valid_, false);

	WISE_ASSERT(config::max_container_size >= len);

	if (config::max_container_size < len)
	{
		WISE_ERROR(
			"unpack vector<T> - size: {} over than maxSize : {%d}",
			len, config::max_container_size
		);

		is_valid_ = false;
		return is_valid_;
	}

	vec.resize(len);

	for (uint16_t i = 0; i < len; ++i)
	{
		is_valid_ = unpack(vec[i]);

		WISE_RETURN_IF(!is_valid_, false);
	}

	WISE_ASSERT(is_valid_);

	return is_valid_;
}

template <class T1, class T2>
inline bool zen_packer::pack(const std::map<T1, T2>& v)
{
	WISE_RETURN_IF(!is_valid_, false);

	WISE_ASSERT(config::max_container_size >= v.size());

	if (config::max_container_size < v.size())
	{
		WISE_ERROR(
			"pack map<T1, T2> - size: {} over than maxSize: {}", 
			v.size(), config::max_container_size
		);

		is_valid_ = false;
		return is_valid_;
	}

	uint16_t len = static_cast<uint16_t>(v.size());

	is_valid_ = pack(len);

	WISE_RETURN_IF(!is_valid_, false);

	for (auto& i : v)
	{
		is_valid_ = pack(const_cast<T1&>(i.first));
		WISE_RETURN_IF(!is_valid_, false);

		is_valid_ = pack(i.second);
		WISE_RETURN_IF(!is_valid_, false);
	}

	WISE_ENSURE(is_valid_);

	return is_valid_;
}

template <class T1, class T2>
inline bool zen_packer::unpack(std::map<T1, T2>& v)
{
	WISE_RETURN_IF(!is_valid_, false);

	uint16_t len = 0;

	is_valid_ = unpack(len);

	WISE_RETURN_IF(!is_valid_, false);

	WISE_ASSERT(len <= config::max_container_size);

	if (config::max_container_size < len)
	{
		WISE_ERROR(
			"unpack map<T1, T2> - size: {} over than maxSize: {}", 
			len, config::max_container_size
		);

		is_valid_ = false;

		return is_valid_;
	}

	for (uint16_t i = 0; i < len; ++i)
	{
		T1 key;

		is_valid_ = unpack(key);
		WISE_RETURN_IF(!is_valid_, false);
		
		T2 value;

		is_valid_ = unpack(value);
		WISE_RETURN_IF(!is_valid_, false);

		is_valid_ = v.emplace(key, value).second;
		WISE_RETURN_IF(!is_valid_, false);
	}

	WISE_ENSURE(is_valid_);

	return is_valid_;
}

template <typename T> 
inline T zen_packer::swap_endian(const T& u)
{
	static_assert(
		std::is_fundamental<T>::value || std::is_enum<T>::value,
		"swapEndian> Type must be fundamental"
		);

	union
	{
		T u;
		unsigned char u8[sizeof(T)];
	} source, dest;

	source.u = u;
	dest.u = u;

	for (size_t k = 0; k < sizeof(T); k++)
		dest.u8[k] = source.u8[sizeof(T) - k - 1];

	return dest.u;
}

} // wise