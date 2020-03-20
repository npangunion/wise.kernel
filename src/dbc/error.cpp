#include "stdafx.h"
#include "error.hpp"

namespace dbc
{
type_incompatible_error::type_incompatible_error()
	: std::runtime_error("type incompatible") { }

type_incompatible_error::type_incompatible_error(const char* desc)
	: std::runtime_error(desc) { }

const char* type_incompatible_error::what() const DBC_NOEXCEPT
{
	return std::runtime_error::what();
}

null_access_error::null_access_error()
	: std::runtime_error("null access") { }

null_access_error::null_access_error(const char* desc)
	: std::runtime_error(desc) { }

const char* null_access_error::what() const DBC_NOEXCEPT
{
	return std::runtime_error::what();
}

index_range_error::index_range_error()
	: std::runtime_error("index out of range") { }

index_range_error::index_range_error(const char* desc)
	: std::runtime_error(desc)
{
}

const char* index_range_error::what() const DBC_NOEXCEPT
{
	return std::runtime_error::what();
}

programming_error::programming_error(const std::string& info)
	: std::runtime_error(info.c_str()) { }

const char* programming_error::what() const DBC_NOEXCEPT
{
	return std::runtime_error::what();
}

database_error::database_error(void* handle, short handle_type, const std::string& info)
	: std::runtime_error(info), native_error(0), sql_state("00000")
{
	message = std::string(std::runtime_error::what()) + recent_error(handle, handle_type, native_error, sql_state);
}

const char* database_error::what() const DBC_NOEXCEPT
{
	return message.c_str();
}

const long database_error::native() const DBC_NOEXCEPT
{
	return native_error;
}

const std::string database_error::state() const DBC_NOEXCEPT
{
	return sql_state;
}

} // namespace dbc