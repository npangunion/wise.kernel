#pragma once 

//! odbc implementation is based on dbc under MIT license.
//!
//! Copyright (C) 2013 lexicalunit <lexicalunit@lexicalunit.com>
//!
//! The MIT License
//!
//! Permission is hereby granted, free of charge, to any person obtaining a copy
//! of this software and associated documentation files (the "Software"), to deal
//! in the Software without restriction, including without limitation the rights
//! to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//! copies of the Software, and to permit persons to whom the Software is
//! furnished to do so, subject to the following conditions:
//!
//! The above copyright notice and this permission notice shall be included in
//! all copies or substantial portions of the Software.
//!
//! THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//! IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//! FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//! AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//! LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//! OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//! THE SOFTWARE.

/// To include winsock through asio.
#include <wise/net/session.hpp>
#include "convert.hpp"

#include <wise/base/date.hpp>
#include <wise/base/macros.hpp>
#include <string>
#include <exception>

namespace dbc
{
#ifndef DOXYGEN

#define DBC_USE_UNICODE 1
// #define DBC_USE_IODBC_WIDE_STRINGS 1

#if defined(_WIN64)
// LLP64 machine: Windows
typedef std::int64_t null_type;
#elif !defined(_WIN64) && defined(__LP64__)
// LP64 machine: OS X or Linux
typedef long null_type;
#else
// 32-bit machine
typedef long null_type;
#endif
#else
//! \c std::wstring will be \c std::u16string or \c std::32string if \c DBC_USE_UNICODE is defined, otherwise \c std::string.
typedef unspecified - type std::wstring;
//! \c null_type will be \c int64_t for 64-bit compilations, otherwise \c long.
typedef unspecified - type null_type;
#endif // DOXYGEN
} // dbc


#if defined(_MSC_VER) && _MSC_VER <= 1800
// These versions of Visual C++ do not yet support \c noexcept or \c std::move.
#define DBC_NOEXCEPT
#define DBC_NO_MOVE_CTOR
#else
#define DBC_NOEXCEPT noexcept
#endif


// User may redefine DBC_ASSERT macro in dbc.h
#ifndef DBC_ASSERT
#include <cassert>
#define DBC_ASSERT(expr) WISE_ASSERT(expr)
#endif

#ifdef DBC_USE_BOOST_CONVERT
#include <boost/locale/encoding_utf.hpp>
#else
#include <codecvt>
#endif

#if defined(_MSC_VER) && _MSC_VER <= 1800
// silence spurious Visual C++ warnings
#pragma warning(disable:4244) // warning about integer conversion issues.
#pragma warning(disable:4312) // warning about 64-bit portability issues.
#pragma warning(disable:4996) // warning about snprintf() deprecated.
#endif

#ifdef __APPLE__
// silence spurious OS X deprecation warnings
#define MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_10_6
#endif

#ifdef _WIN32
// needs to be included above sql.h for windows
#ifndef NOMINMAX
#define NOMINMAX
#endif 

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#endif

// disable async functions, which not compatible with windows 7 driver
#define ODBCVER 0x0351

#include <sql.h>
#include <sqlext.h>

// Default to ODBC version defined by DBC_ODBC_VERSION if provided.
#ifndef DBC_ODBC_VERSION
#ifdef SQL_OV_ODBC3_80
// Otherwise, use ODBC v3.8 if it's available...
#define DBC_ODBC_VERSION SQL_OV_ODBC3_80
#else
// or fallback to ODBC v3.x.
#define DBC_ODBC_VERSION SQL_OV_ODBC3
#endif
#endif


#ifdef DBC_USE_UNICODE
#define DBC_TEXT(s) L ## s
#define DBC_FUNC(f) f ## W
#define DBC_SQLCHAR SQLWCHAR
#else
#define DBC_TEXT(s) s
#define DBC_FUNC(f) f
#define DBC_SQLCHAR SQLCHAR
#endif

#if defined(_MSC_VER)
#ifndef DBC_USE_UNICODE
// Disable unicode in sqlucode.h on Windows when DBC_USE_UNICODE
// is not defined. This is required because unicode is enabled by
// default on many Windows systems.
#define SQL_NOUNICODEMAP
#endif
#endif

#define DBC_STRINGIZE_I(text) #text
#define DBC_STRINGIZE(text) DBC_STRINGIZE_I(text)

// By making all calls to ODBC functions through this macro, we can easily get
// runtime debugging information of which ODBC functions are being called,
// in what order, and with what parameters by defining DBC_ODBC_API_DEBUG.
#ifdef DBC_ODBC_API_DEBUG
#include <iostream>
#define DBC_CALL_RC(FUNC, RC, ...)                                    \
        do                                                                    \
        {                                                                     \
            std::cerr << __FILE__ ":" DBC_STRINGIZE(__LINE__) " "         \
                DBC_STRINGIZE(FUNC) "(" #__VA_ARGS__ ")" << std::endl;    \
            RC = FUNC(__VA_ARGS__);                                           \
        } while(false)                                                        \
        /**/
#define DBC_CALL(FUNC, ...)                                           \
        do                                                                    \
        {                                                                     \
            std::cerr << __FILE__ ":" DBC_STRINGIZE(__LINE__) " "         \
                DBC_STRINGIZE(FUNC) "(" #__VA_ARGS__ ")" << std::endl;    \
            FUNC(__VA_ARGS__);                                                \
        } while(false)                                                        \
        /**/
#else
#define DBC_CALL_RC(FUNC, RC, ...) RC = FUNC(__VA_ARGS__)
#define DBC_CALL(FUNC, ...) FUNC(__VA_ARGS__)
#endif


namespace dbc
{

template<class T>
struct sql_ctype { };

template<>
struct sql_ctype<std::wstring::value_type>
{
	static const SQLSMALLINT value = SQL_C_WCHAR;
};

template<>
struct sql_ctype<std::string::value_type>
{
	static const SQLSMALLINT value = SQL_C_CHAR;
};

template<>
struct sql_ctype<bool>
{
	static const SQLSMALLINT value = SQL_C_BIT;
};

template<>
struct sql_ctype<unsigned char>
{
	static const SQLSMALLINT value = SQL_C_CHAR;
};

template<>
struct sql_ctype<short>
{
	static const SQLSMALLINT value = SQL_C_SSHORT;
};

template<>
struct sql_ctype<unsigned short>
{
	static const SQLSMALLINT value = SQL_C_USHORT;
};

template<>
struct sql_ctype<int32_t>
{
	static const SQLSMALLINT value = SQL_C_SLONG;
};

template<>
struct sql_ctype<uint32_t>
{
	static const SQLSMALLINT value = SQL_C_ULONG;
};

template<>
struct sql_ctype<int64_t>
{
	static const SQLSMALLINT value = SQL_C_SBIGINT;
};

template<>
struct sql_ctype<uint64_t>
{
	static const SQLSMALLINT value = SQL_C_UBIGINT;
};

template<>
struct sql_ctype<float>
{
	static const SQLSMALLINT value = SQL_C_FLOAT;
};

template<>
struct sql_ctype<double>
{
	static const SQLSMALLINT value = SQL_C_DOUBLE;
};

template<>
struct sql_ctype<std::wstring>
{
	static const SQLSMALLINT value = SQL_C_WCHAR;
};

template<>
struct sql_ctype<std::string>
{
	static const SQLSMALLINT value = SQL_C_CHAR;
};

using date = wise::date;
using timestamp = wise::timestamp;

template<>
struct sql_ctype<dbc::date>
{
	static const SQLSMALLINT value = SQL_C_DATE;
};

template<>
struct sql_ctype<dbc::timestamp>
{
	static const SQLSMALLINT value = SQL_C_TYPE_TIMESTAMP;
};

// Allocates the native ODBC handles.
void allocate_handle(SQLHENV& env, SQLHDBC& conn);

} // dbc