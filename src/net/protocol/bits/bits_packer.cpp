#include "stdafx.h"
#include "zen_packer.hpp"

// 표준이 안정될 때까지 변환 기능만 사용
#include <wise/base/utf8.h>

namespace wise
{

bool zen_packer::convert(const std::wstring& src, std::vector<uint8_t>& out)
{
	try
	{
		utf8::utf16to8(src.begin(), src.end(), std::back_inserter(out));
	}
	catch (std::exception& exp)
	{
		WISE_ERROR("exception: {}", exp.what());
		is_valid_ = false;
		return is_valid_;
	}

	return is_valid_;
}

bool zen_packer::convert(const std::vector<uint8_t>& src, std::wstring& out)
{
	try
	{
		utf8::utf8to16(src.begin(), src.end(), std::back_inserter(out));
	}
	catch (std::exception& exp)
	{
		WISE_ERROR("exception: {}", exp.what());
		is_valid_ = false;
		return is_valid_;
	}

	return is_valid_;
}

} // wise