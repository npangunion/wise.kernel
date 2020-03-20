#include <pch.hpp>

#include <wise.kernel/dbc/convert.hpp>
#include <wise.kernel/core/macros.hpp>
#include <algorithm>

namespace wise {
namespace kernel {

#ifdef _MSC_VER

#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>

bool convert(const std::string& src, std::string& out, int32_t code_page)
{
	std::wstring wstr;
	convert(src, wstr, code_page);
	convert(wstr, out, code_page);

	return true;
}

bool convert(const std::wstring& src, std::wstring& out, int32_t code_page)
{
	WISE_UNUSED(code_page);

	out = src;
	return true;
}

bool convert(const wchar_t* src, std::wstring& out, int32_t code_page)
{
	WISE_UNUSED(code_page);

	out = src;
	return true;
}

bool convert(const char* src, std::wstring& out, int32_t code_page)
{
	out.clear();

	int slen = std::min<int>(8192, (int)strlen(src));
	if (slen == 0)
	{
		return true;
	}

	int wlen = MultiByteToWideChar(code_page, 0, src, slen, NULL, NULL);
	LPWSTR np = (LPWSTR)_alloca(sizeof(WCHAR) * (wlen + 1)); // 스택에서 할당
	::memset(np, 0, (wlen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(code_page, 0, src, slen, np, wlen);

	out.append(np);

	return true;
}


bool convert(const std::string& src, std::wstring& out, int32_t code_page)
{
	out.clear();

	int slen = std::min<int>(8192, (int)strlen(src.c_str()));
	if (slen == 0)
	{
		return true;
	}

	int wlen = MultiByteToWideChar(code_page, 0, src.c_str(), slen, NULL, NULL);
	LPWSTR np = (LPWSTR)_alloca(sizeof(WCHAR) * (wlen + 1)); // 스택에서 할당
	::memset(np, 0, (wlen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(code_page, 0, src.c_str(), slen, np, wlen);

	out.append(np);

	return true;
}

bool convert(const std::wstring& s, std::string& out, int32_t code_page)
{
	out.clear();

	int slen = std::min<int>(8192, (int)wcslen(s.c_str()));

	int wlen = WideCharToMultiByte(code_page, 0, s.c_str(), slen, NULL, 0, NULL, NULL);
	if (wlen == 0)
	{
		return true;
	}

	LPSTR np = (LPSTR)_alloca(sizeof(CHAR) * (wlen + 1));
	::memset(np, 0, (wlen + 1) * sizeof(CHAR));
	WideCharToMultiByte(code_page, 0, s.c_str(), slen, np, wlen, NULL, NULL);

	out.append(np);

	return true;
}

#endif // windows

} // kernel
} // wise