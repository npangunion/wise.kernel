#pragma once 

#include "common.hpp"

#include <algorithm>
#include <vector>

namespace dbc
{

//! \brief Type incompatible.
//! \see exceptions
class type_incompatible_error : public std::runtime_error
{
public:
	type_incompatible_error();
	type_incompatible_error(const char* desc);
	const char* what() const DBC_NOEXCEPT;
};

//! \brief Accessed null data.
//! \see exceptions
class null_access_error : public std::runtime_error
{
public:
	null_access_error();
	null_access_error(const char* desc);
	const char* what() const DBC_NOEXCEPT;
};

//! \brief Index out of range.
//! \see exceptions
class index_range_error : public std::runtime_error
{
public:
	index_range_error();
	index_range_error(const char* desc);
	const char* what() const DBC_NOEXCEPT;
};

//! \brief Programming logic error.
//! \see exceptions
class programming_error : public std::runtime_error
{
public:
	explicit programming_error(const std::string& info);
	const char* what() const DBC_NOEXCEPT;
};

//! \brief General database error.
//! \see exceptions
class database_error : public std::runtime_error
{
public:
	//! \brief Creates a runtime_error with a message describing the last ODBC error generated for the given handle and handle_type.
	//! \param handle The native ODBC statement or connection handle.
	//! \param handle_type The native ODBC handle type code for the given handle.
	//! \param info Additional information that will be appended to the beginning of the error message.
	database_error(void* handle, short handle_type, const std::string& info = "");
	const char* what() const DBC_NOEXCEPT;
	const long native() const DBC_NOEXCEPT;
	const std::string state() const DBC_NOEXCEPT;
private:
	long native_error;
	std::string sql_state;
	std::string message;
};


#ifdef DBC_ODBC_API_DEBUG
inline nanodbc::std::wstring return_code(RETCODE rc)
{
	switch (rc)
	{
	case SQL_SUCCESS: return DBC_TEXT("SQL_SUCCESS");
	case SQL_SUCCESS_WITH_INFO: return DBC_TEXT("SQL_SUCCESS_WITH_INFO");
	case SQL_ERROR: return DBC_TEXT("SQL_ERROR");
	case SQL_INVALID_HANDLE: return DBC_TEXT("SQL_INVALID_HANDLE");
	case SQL_NO_DATA: return DBC_TEXT("SQL_NO_DATA");
	case SQL_NEED_DATA: return DBC_TEXT("SQL_NEED_DATA");
	case SQL_STILL_EXECUTING: return DBC_TEXT("SQL_STILL_EXECUTING");
	}
	DBC_ASSERT(0);
	return "unknown"; // should never make it here
}
#endif

// Easy way to check if a return code signifies success.
inline bool success(RETCODE rc)
{
#ifdef DBC_ODBC_API_DEBUG
	std::cerr << "<-- rc: " << return_code(rc) << " | ";
#endif
	return rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO;
}

// Returns the array size.
template<typename T, std::size_t N>
inline std::size_t arrlen(T(&)[N])
{
	return N;
}

// Operates like strlen() on a character array.
template<typename T, std::size_t N>
inline std::size_t strarrlen(T(&a)[N])
{
	const T* s = &a[0];
	std::size_t i = 0;
	while (*s++ && i < N) i++;
	return i;
}

inline void convert(const std::wstring& in, std::string& out)
{
#ifdef DBC_USE_BOOST_CONVERT
	using boost::locale::conv::utf_to_utf;
	out = utf_to_utf<char>(in.c_str(), in.c_str() + in.size());
#elif defined(_MSC_VER)
	wise::kernel::convert(in, out);
#else 
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;

	out = converter.to_bytes(in);
#endif
}

inline void convert(const std::string& in, std::wstring& out)
{
#ifdef DBC_USE_BOOST_CONVERT
	using boost::locale::conv::utf_to_utf;
	out = utf_to_utf<wide_char_t>(in.c_str(), in.c_str() + in.size());
#elif defined(_MSC_VER)
	wise::kernel::convert(in, out);
#else
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;

	out = converter.from_bytes(in);
#endif
}

inline void convert(const std::wstring& in, std::wstring& out)
{
	out = in;
}

inline void convert(const std::string& in, std::string& out)
{
	out = in;
}

// Attempts to get the most recent ODBC error as a string.
// Always returns std::string, even in unicode mode.
std::string recent_error(
	SQLHANDLE handle
	, SQLSMALLINT handle_type
	, long& native
	, std::string& state);

} // dbc

  // Throwing exceptions using DBC_THROW_DATABASE_ERROR enables file name
  // and line numbers to be inserted into the error message. Useful for debugging.
#define DBC_THROW_DATABASE_ERROR(handle, handle_type)                     \
    throw dbc::database_error(                                            \
        handle                                                            \
        , handle_type                                                     \
        , __FILE__ ":" DBC_STRINGIZE(__LINE__) ": ")                      \
    /**/