#include "stdafx.h"

#include "common.hpp"
#include "error.hpp"

namespace dbc
{

// Allocates the native ODBC handles.
void allocate_handle(SQLHENV& env, SQLHDBC& conn)
{
	RETCODE rc;
	DBC_CALL_RC(
		SQLAllocHandle
		, rc
		, SQL_HANDLE_ENV
		, SQL_NULL_HANDLE
		, &env);

	if (!success(rc))
	{
		DBC_THROW_DATABASE_ERROR(env, SQL_HANDLE_ENV);
	}

	try
	{
		DBC_CALL_RC(
			SQLSetEnvAttr
			, rc
			, env
			, SQL_ATTR_ODBC_VERSION
			, (SQLPOINTER)DBC_ODBC_VERSION
			, SQL_IS_UINTEGER);
		if (!success(rc))
			DBC_THROW_DATABASE_ERROR(env, SQL_HANDLE_ENV);

		DBC_CALL_RC(
			SQLAllocHandle
			, rc
			, SQL_HANDLE_DBC
			, env
			, &conn);
		if (!success(rc))
			DBC_THROW_DATABASE_ERROR(env, SQL_HANDLE_ENV);
	}
	catch (...)
	{
		DBC_CALL(
			SQLFreeHandle
			, SQL_HANDLE_ENV
			, env);
		throw;
	}
}

} // dbc 