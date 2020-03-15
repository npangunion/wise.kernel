#include <pch.hpp>
#include <wise.kernel/net/protocol/bits/bits_packer.hpp>

// ǥ���� ������ ������ ��ȯ ��ɸ� ���
#include <wise.kernel/util/utf8.h>

namespace wise {
namespace kernel {

bool bits_packer::convert(const std::wstring& src, std::vector<uint8_t>& out)
{
	try
	{
		utf8::utf16to8(src.begin(), src.end(), std::back_inserter(out));
	}
	catch (std::exception & exp)
	{
		WISE_ERROR("exception: {}", exp.what());
		is_valid_ = false;
		return is_valid_;
	}

	return is_valid_;
}

bool bits_packer::convert(const std::vector<uint8_t>& src, std::wstring& out)
{
	try
	{
		utf8::utf8to16(src.begin(), src.end(), std::back_inserter(out));
	}
	catch (std::exception & exp)
	{
		WISE_ERROR("exception: {}", exp.what());
		is_valid_ = false;
		return is_valid_;
	}

	return is_valid_;
}

} // kernel
} // wise