#pragma once

#include <string>

namespace wise
{

// conversions. implemented only on windows
bool convert(const std::string& src, std::wstring& out, int32_t code_page = 0);
bool convert(const std::string& src, std::string& out, int32_t code_page = 0);
bool convert(const std::wstring& src, std::string& out, int32_t code_page = 0);
bool convert(const std::wstring& src, std::wstring& out, int32_t code_page = 0);
bool convert(const wchar_t* src, std::wstring& out, int32_t code_page = 0);
bool convert(const char* src, std::wstring& out, int32_t code_page = 0);

} // wise