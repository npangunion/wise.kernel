#pragma once 

#include "common.hpp"
#include <memory>

namespace dbc
{

class transaction;

//! \brief Manages and encapsulates ODBC resources such as the connection and environment handles.
class connection
{
public: 
	using ptr = std::shared_ptr<connection>;

public:
	//! \brief Create new connection object, initially not connected.
	connection();

	//! \brief Create new connection object and immediately connect to the given data source.
	//! \param dsn The name of the data source.
	//! \param user The username for authenticating to the data source.
	//! \param pass The password for authenticating to the data source.
	//! \param timeout The number in seconds before connection timeout. Default is 0 indicating no timeout.
	//! \throws database_error
	//! \see connected(), connect()
	connection(
		const std::wstring& dsn
		, const std::wstring& user
		, const std::wstring& pass
		, long timeout = 0);

	//! \brief Create new connection object and immediately connect using the given connection string.
	//! \param connection_string The connection string for establishing a connection.
	//! \param timeout The number in seconds before connection timeout. Default is 0 indicating no timeout.
	//! \throws database_error
	//! \see connected(), connect()
	connection(const std::wstring& connection_string, long timeout = 0);

	//! \brief Automatically disconnects from the database and frees all associated resources.
	//!
	//! Will not throw even if disconnecting causes some kind of error and raises an exception.
	//! If you explicitly need to know if disconnect() succeeds, call it directly.
	~connection() DBC_NOEXCEPT;

	//! \brief Connect to the given data source.
	//! \param dsn The name of the data source.
	//! \param user The username for authenticating to the data source.
	//! \param pass The password for authenticating to the data source.
	//! \param timeout The number in seconds before connection timeout. Default is 0 indicating no timeout.
	//! \throws database_error
	//! \see connected()
	void connect(
		const std::wstring& dsn
		, const std::wstring& user
		, const std::wstring& pass
		, long timeout = 0);

	//! \brief Connect using the given connection string.
	//! \param connection_string The connection string for establishing a connection.
	//! \param timeout The number in seconds before connection timeout. Default is 0 indicating no timeout.
	//! \throws database_error
	//! \see connected()
	void connect(const std::wstring& connection_string, long timeout = 0);

	// async API needs to have some time to fully supported

	//! \brief Returns true if connected to the database.
	bool connected() const;

	//! \brief Disconnects from the database, but maintains environment and handle resources.
	void disconnect();

	//! \brief Returns the number of transactions currently held for this connection.
	std::size_t transactions() const;

	//! \brief Returns the native ODBC database connection handle.
	void* native_dbc_handle() const;

	//! \brief Returns the native ODBC environment handle.
	void* native_env_handle() const;

	//! \brief Returns name of the DBMS product.
	//! Returns the ODBC information type SQL_DBMS_NAME of the DBMS product
	//! accesssed by the driver via the current connection.
	std::wstring dbms_name() const;

	//! \brief Returns version of the DBMS product.
	//! Returns the ODBC information type SQL_DBMS_VER of the DBMS product
	//! accesssed by the driver via the current connection.
	std::wstring dbms_version() const;

	//! \brief Returns the name of the ODBC driver.
	//! \throws database_error
	std::wstring driver_name() const;

	//! \brief Returns the name of the currently connected database.
	//! Returns the current SQL_DATABASE_NAME information value associated with the connection.
	std::wstring database_name() const;

	//! \brief Returns the name of the current catalog.
	//! Returns the current setting of the connection attribute SQL_ATTR_CURRENT_CATALOG.
	std::wstring catalog_name() const;

	// no move around
	connection(const connection& rhs) = delete;
	connection(connection&& rhs) = delete;
	connection& operator=(connection rhs) = delete;

private:
	std::size_t ref_transaction();
	std::size_t unref_transaction();
	bool rollback() const;
	void rollback(bool onoff);

private:
	class connection_impl;
	friend class transaction;

private:
	std::shared_ptr<connection_impl> impl_;
};

} // dbc