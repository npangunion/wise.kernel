#include <pch.hpp>
#include <wise.kernel/dbc/connection.hpp>
#include <wise.kernel/dbc/error.hpp>

namespace dbc
{

class connection::connection_impl
{
public:
	connection_impl(const connection_impl&) = delete;
	connection_impl& operator=(const connection_impl&) = delete;

	connection_impl()
		: env_(0)
		, conn_(0)
		, connected_(false)
		, transactions_(0)
		, rollback_(false)
	{
		allocate_handle(env_, conn_);
	}

	connection_impl(
		const std::wstring& dsn
		, const std::wstring& user
		, const std::wstring& pass
		, long timeout)
		: env_(0)
		, conn_(0)
		, connected_(false)
		, transactions_(0)
		, rollback_(false)
	{
		allocate_handle(env_, conn_);
		try
		{
			connect(dsn, user, pass, timeout);
		}
		catch (...)
		{
			DBC_CALL(
				SQLFreeHandle
				, SQL_HANDLE_DBC
				, conn_);
			DBC_CALL(
				SQLFreeHandle
				, SQL_HANDLE_ENV
				, env_);
			throw;
		}
	}

	connection_impl(const std::wstring& connection_string, long timeout)
		: env_(0)
		, conn_(0)
		, connected_(false)
		, transactions_(0)
		, rollback_(false)
	{
		allocate_handle(env_, conn_);

		try
		{
			connect(connection_string, timeout);
		}
		catch (...)
		{
			DBC_CALL(
				SQLFreeHandle
				, SQL_HANDLE_DBC
				, conn_);
			DBC_CALL(
				SQLFreeHandle
				, SQL_HANDLE_ENV
				, env_);
			throw;
		}
	}

	~connection_impl() DBC_NOEXCEPT
	{
		try
		{
			disconnect();
		}
		catch (...)
		{
			// ignore exceptions thrown during disconnect
		}

		DBC_CALL(
			SQLFreeHandle
			, SQL_HANDLE_DBC
			, conn_);
		DBC_CALL(
			SQLFreeHandle
			, SQL_HANDLE_ENV
			, env_);
	}

	void connect(
		const std::wstring& dsn
		, const std::wstring& user
		, const std::wstring& pass
		, long timeout
		, void* event_handle = NULL)
	{
		disconnect();

		RETCODE rc;
		DBC_CALL_RC(
			SQLFreeHandle
			, rc
			, SQL_HANDLE_DBC
			, conn_);
		if (!success(rc))
			DBC_THROW_DATABASE_ERROR(conn_, SQL_HANDLE_DBC);

		DBC_CALL_RC(
			SQLAllocHandle
			, rc
			, SQL_HANDLE_DBC
			, env_
			, &conn_);
		if (!success(rc))
			DBC_THROW_DATABASE_ERROR(env_, SQL_HANDLE_ENV);

		DBC_CALL_RC(
			SQLSetConnectAttr
			, rc
			, conn_
			, SQL_LOGIN_TIMEOUT
			, (SQLPOINTER)(std::intptr_t)timeout
			, 0);
		if (!success(rc))
			DBC_THROW_DATABASE_ERROR(conn_, SQL_HANDLE_DBC);

#ifdef SQL_ATTR_ASYNC_DBC_EVENT
		if (event_handle != NULL)
			enable_async(event_handle);
#endif

		DBC_CALL_RC(
			DBC_FUNC(SQLConnect)
			, rc
			, conn_
			, (DBC_SQLCHAR*)dsn.c_str(), SQL_NTS
			, !user.empty() ? (DBC_SQLCHAR*)user.c_str() : 0, SQL_NTS
			, !pass.empty() ? (DBC_SQLCHAR*)pass.c_str() : 0, SQL_NTS);
		if (!success(rc) && (event_handle == NULL || rc != SQL_STILL_EXECUTING))
			DBC_THROW_DATABASE_ERROR(conn_, SQL_HANDLE_DBC);

		connected_ = success(rc);
	}

	void connect(const std::wstring& connection_string, long timeout, void* event_handle = NULL)
	{
		disconnect();

		RETCODE rc;
		DBC_CALL_RC(
			SQLFreeHandle
			, rc
			, SQL_HANDLE_DBC
			, conn_);
		if (!success(rc))
			DBC_THROW_DATABASE_ERROR(conn_, SQL_HANDLE_DBC);

		DBC_CALL_RC(
			SQLAllocHandle
			, rc
			, SQL_HANDLE_DBC
			, env_
			, &conn_);
		if (!success(rc))
			DBC_THROW_DATABASE_ERROR(env_, SQL_HANDLE_ENV);

		DBC_CALL_RC(
			SQLSetConnectAttr
			, rc
			, conn_
			, SQL_LOGIN_TIMEOUT
			, (SQLPOINTER)(std::intptr_t)timeout
			, 0);
		if (!success(rc))
			DBC_THROW_DATABASE_ERROR(conn_, SQL_HANDLE_DBC);

#ifdef SQL_ATTR_ASYNC_DBC_EVENT
		if (event_handle != NULL)
			enable_async(event_handle);
#endif

		DBC_SQLCHAR dsn[1024];
		SQLSMALLINT dsn_size = 0;
		DBC_CALL_RC(
			DBC_FUNC(SQLDriverConnect)
			, rc
			, conn_
			, 0
			, (DBC_SQLCHAR*)connection_string.c_str(), SQL_NTS
			, dsn
			, sizeof(dsn) / sizeof(DBC_SQLCHAR)
			, &dsn_size
			, SQL_DRIVER_NOPROMPT);
		if (!success(rc) && (event_handle == NULL || rc != SQL_STILL_EXECUTING))
			DBC_THROW_DATABASE_ERROR(conn_, SQL_HANDLE_DBC);

		connected_ = success(rc);
	}

	bool connected() const
	{
		return connected_;
	}

	void disconnect()
	{
		if (connected())
		{
			RETCODE rc;
			DBC_CALL_RC(
				SQLDisconnect
				, rc
				, conn_);
			if (!success(rc))
				DBC_THROW_DATABASE_ERROR(conn_, SQL_HANDLE_DBC);
		}
		connected_ = false;
	}

	std::size_t transactions() const
	{
		return transactions_;
	}

	void* native_dbc_handle() const
	{
		return conn_;
	}

	void* native_env_handle() const
	{
		return env_;
	}

	std::wstring dbms_name() const
	{
		DBC_SQLCHAR name[255] = { 0 };
		SQLSMALLINT length(0);
		RETCODE rc;
		DBC_CALL_RC(
			DBC_FUNC(SQLGetInfo)
			, rc
			, conn_
			, SQL_DBMS_NAME
			, name
			, sizeof(name) / sizeof(DBC_SQLCHAR)
			, &length);
		if (!success(rc))
			DBC_THROW_DATABASE_ERROR(conn_, SQL_HANDLE_DBC);
		return std::wstring(&name[0], &name[strarrlen(name)]);
	}

	std::wstring dbms_version() const
	{
		DBC_SQLCHAR version[255] = { 0 };
		SQLSMALLINT length(0);
		RETCODE rc;
		DBC_CALL_RC(
			DBC_FUNC(SQLGetInfo)
			, rc
			, conn_
			, SQL_DBMS_VER
			, version
			, sizeof(version) / sizeof(DBC_SQLCHAR)
			, &length);
		if (!success(rc))
			DBC_THROW_DATABASE_ERROR(conn_, SQL_HANDLE_DBC);
		return std::wstring(&version[0], &version[strarrlen(version)]);
	}

	std::wstring driver_name() const
	{
		DBC_SQLCHAR name[1024];
		SQLSMALLINT length;
		RETCODE rc;
		DBC_CALL_RC(
			DBC_FUNC(SQLGetInfo)
			, rc
			, conn_
			, SQL_DRIVER_NAME
			, name
			, sizeof(name) / sizeof(DBC_SQLCHAR)
			, &length);
		if (!success(rc))
			DBC_THROW_DATABASE_ERROR(conn_, SQL_HANDLE_DBC);
		return std::wstring(&name[0], &name[strarrlen(name)]);
	}

	std::wstring database_name() const
	{
		// FIXME: Allocate buffer of dynamic size as drivers do not agree on universal size
		// MySQL driver limits MAX_NAME_LEN=255
		// PostgreSQL driver MAX_INFO_STIRNG=128
		// MFC CDatabase allocates buffer dynamically.
		DBC_SQLCHAR name[255] = { 0 };
		SQLSMALLINT length(0);
		RETCODE rc;
		DBC_CALL_RC(
			DBC_FUNC(SQLGetInfo)
			, rc
			, conn_
			, SQL_DATABASE_NAME
			, name
			, sizeof(name) / sizeof(DBC_SQLCHAR)
			, &length);
		if (!success(rc))
			DBC_THROW_DATABASE_ERROR(conn_, SQL_HANDLE_DBC);
		return std::wstring(&name[0], &name[strarrlen(name)]);
	}

	std::wstring catalog_name() const
	{
		DBC_SQLCHAR name[SQL_MAX_OPTION_STRING_LENGTH] = { 0 };
		SQLINTEGER length(0);
		RETCODE rc;
		DBC_CALL_RC(
			DBC_FUNC(SQLGetConnectAttr)
			, rc
			, conn_
			, SQL_ATTR_CURRENT_CATALOG
			, name
			, sizeof(name) / sizeof(DBC_SQLCHAR)
			, &length);
		if (!success(rc))
			DBC_THROW_DATABASE_ERROR(conn_, SQL_HANDLE_DBC);
		return std::wstring(&name[0], &name[strarrlen(name)]);
	}


	std::size_t ref_transaction()
	{
		return --transactions_;
	}

	std::size_t unref_transaction()
	{
		return ++transactions_;
	}

	bool rollback() const
	{
		return rollback_;
	}

	void rollback(bool onoff)
	{
		rollback_ = onoff;
	}

private:
	HENV env_;
	HDBC conn_;
	bool connected_;
	std::size_t transactions_;
	bool rollback_; // if true, this connection is marked for eventual transaction rollback
};

connection::connection()
	: impl_(new connection_impl())
{

}

connection::connection(
	const std::wstring& dsn
	, const std::wstring& user
	, const std::wstring& pass
	, long timeout)
	: impl_(new connection_impl(dsn, user, pass, timeout))
{

}

connection::connection(const std::wstring& connection_string, long timeout)
	: impl_(new connection_impl(connection_string, timeout))
{

}

connection::~connection() DBC_NOEXCEPT
{

}

void connection::connect(
	const std::wstring& dsn
	, const std::wstring& user
	, const std::wstring& pass
	, long timeout)
{
	impl_->connect(dsn, user, pass, timeout);
}

void connection::connect(const std::wstring& connection_string, long timeout)
{
	impl_->connect(connection_string, timeout);
}

bool connection::connected() const
{
	return impl_->connected();
}

void connection::disconnect()
{
	impl_->disconnect();
}

std::size_t connection::transactions() const
{
	return impl_->transactions();
}

void* connection::native_dbc_handle() const
{
	return impl_->native_dbc_handle();
}

void* connection::native_env_handle() const
{
	return impl_->native_env_handle();
}

std::wstring connection::dbms_name() const
{
	return impl_->dbms_name();
}

std::wstring connection::dbms_version() const
{
	return impl_->dbms_version();
}

std::wstring connection::driver_name() const
{
	return impl_->driver_name();
}

std::wstring connection::database_name() const
{
	return impl_->database_name();
}

std::wstring connection::catalog_name() const
{
	return impl_->catalog_name();
}

std::size_t connection::ref_transaction()
{
	return impl_->ref_transaction();
}

std::size_t connection::unref_transaction()
{
	return impl_->unref_transaction();
}

bool connection::rollback() const
{
	return impl_->rollback();
}

void connection::rollback(bool onoff)
{
	impl_->rollback(onoff);
}



} // dbc