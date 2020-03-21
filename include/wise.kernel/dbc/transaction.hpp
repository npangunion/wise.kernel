#pragma once 

#include "common.hpp"

#include <memory>

namespace dbc
{

class connection;

//! \brief A resource for managing transaction commits and rollbacks.
//!
//! \attention You will want to use transactions if you are doing batch operations because 
//! it will prevent auto commits from occurring after each individual operation is executed.
class transaction
{
public:
	//! \brief Begin a transaction on the given connection object.
	//! \post Operations that modify the database must now be committed before taking effect.
	//! \throws database_error
	explicit transaction(connection* conn);

	//! \brief If this transaction has not been committed, will will rollback any modifying operations.
	~transaction() DBC_NOEXCEPT;

	//! \brief Marks this transaction for commit.
	//! \throws database_error
	void commit();

	//! \brief Marks this transaction for rollback.
	void rollback() DBC_NOEXCEPT;

	//! Returns the connection object.
	connection* get_connection();

	//! Returns the connection object.
	const connection* get_connection() const;

	// no move around
	transaction(const transaction& rhs) = delete;
	transaction(transaction&& rhs) = delete;
	transaction& operator=(transaction rhs) = delete;

private: 
	// forward call to connection for impl
	std::size_t ref_transaction();
	std::size_t unref_transaction();
	bool rollback() const;
	void rollback(bool onoff);

private:
	// remvoe pimpl
	class transaction_impl;
	std::shared_ptr<transaction_impl> impl_;
};

} // dbc