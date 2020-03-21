#include <pch.hpp>
#include <wise.kernel/dbc/error.hpp>
#include <wise.kernel/dbc/convert.hpp>

namespace dbc
{

std::string recent_error(
	SQLHANDLE handle
	, SQLSMALLINT handle_type
	, long& native
	, std::string& state)
{
	std::wstring result;
	std::string rvalue;
	std::vector<DBC_SQLCHAR> sql_message(SQL_MAX_MESSAGE_LENGTH);
	sql_message[0] = '\0';

	SQLINTEGER i = 1;
	SQLINTEGER native_error;
	SQLSMALLINT total_bytes;
	DBC_SQLCHAR sql_state[6];
	RETCODE rc;

	do
	{
		DBC_CALL_RC(
			DBC_FUNC(SQLGetDiagRec)
			, rc
			, handle_type
			, handle
			, (SQLSMALLINT)i
			, sql_state
			, &native_error
			, 0
			, 0
			, &total_bytes);

		if (success(rc) && total_bytes > 0)
			sql_message.resize(total_bytes + 1);

		if (rc == SQL_NO_DATA)
			break;

		DBC_CALL_RC(
			DBC_FUNC(SQLGetDiagRec)
			, rc
			, handle_type
			, handle
			, (SQLSMALLINT)i
			, sql_state
			, &native_error
			, sql_message.data()
			, (SQLSMALLINT)sql_message.size()
			, &total_bytes);

		if (!success(rc))
		{
			convert(result, rvalue);
			return rvalue;
		}

		if (!result.empty())
			result += ' ';

		result += std::wstring(sql_message.begin(), sql_message.end());
		i++;

		// NOTE: unixODBC using PostgreSQL and SQLite drivers crash if you call SQLGetDiagRec()
		// more than once. So as a (terrible but the best possible) workaround just exit
		// this loop early on non-Windows systems.
#ifndef _MSC_VER
		break;
#endif
	} while (rc != SQL_NO_DATA);

	convert(result, rvalue);
	convert(sql_state, state);

	native = native_error;
	std::string status = state;
	status += ": ";
	status += rvalue;

	// some drivers insert \0 into error messages for unknown reasons
	using std::replace;
	replace(status.begin(), status.end(), '\0', ' ');

	return status;
}

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