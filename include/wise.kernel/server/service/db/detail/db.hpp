#pragma once

#include <wise/dbc/connection.hpp>

#include <spdlog/fmt/fmt.h>
#include <string>

namespace wise
{

/// db information
class db 
{
public: 
	struct config
	{
		std::string key;	// db key
		std::wstring host;
		std::wstring name; 
		std::wstring user; 
		std::wstring password;
		std::wstring database;
	};

public: 
	/// constructor. creates dsn
	db(const config& cfg);

	/// copy 
	db(const db& rhs);
	db& operator=(const db& rhs);

	/// connect to db. reconnect if connected.
	bool connect();

	/// get dsn
	const std::wstring& get_dsn() const
	{
		return dsn_;
	}

	/// get config
	const config& get_config() const
	{
		return config_;
	}

	/// get connection
	dbc::connection::ptr get_connection()
	{
		return conn_;
	}

private: 
	config config_;
	std::wstring dsn_;
	dbc::connection::ptr conn_;
};

} // wise