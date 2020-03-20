#include "stdafx.h"
#include "transaction.hpp"
#include "connection.hpp"
#include "error.hpp"

namespace dbc
{

class transaction::transaction_impl
{
public:
	transaction_impl(const transaction_impl&) = delete;
	transaction_impl& operator=(const transaction_impl&) = delete;

	transaction_impl(transaction* tran, connection* conn)
		: tran_(tran)
		, conn_(conn)
		, committed_(false)
	{
		WISE_ASSERT(conn);

		if (conn_->transactions() == 0 && conn_->connected())
		{
			RETCODE rc;
			DBC_CALL_RC(
				SQLSetConnectAttr
				, rc
				, conn_->native_dbc_handle()
				, SQL_ATTR_AUTOCOMMIT
				, (SQLPOINTER)SQL_AUTOCOMMIT_OFF
				, SQL_IS_UINTEGER);

			if (!success(rc))
			{
				DBC_THROW_DATABASE_ERROR(conn_->native_dbc_handle(), SQL_HANDLE_DBC);
			}
		}

		tran_->ref_transaction();
	}

	~transaction_impl() DBC_NOEXCEPT
	{
		if (!committed_)
		{
			tran_->rollback(true);
			tran_->unref_transaction();
		}

		if (conn_->transactions() == 0 && conn_->connected())
		{
			if (conn_->rollback())
			{
				DBC_CALL(
					SQLEndTran
					, SQL_HANDLE_DBC
					, conn_->native_dbc_handle()
					, SQL_ROLLBACK);
				tran_->rollback(false);
			}

			DBC_CALL(
				SQLSetConnectAttr
				, conn_->native_dbc_handle()
				, SQL_ATTR_AUTOCOMMIT
				, (SQLPOINTER)SQL_AUTOCOMMIT_ON
				, SQL_IS_UINTEGER);
		}
	}

	void commit()
	{
		if (committed_)
		{
			return;
		}

		committed_ = true;

		if (tran_->unref_transaction() == 0 && conn_->connected())
		{
			RETCODE rc;
			DBC_CALL_RC(
				SQLEndTran
				, rc
				, SQL_HANDLE_DBC
				, conn_->native_dbc_handle()
				, SQL_COMMIT);

			if (!success(rc))
			{
				DBC_THROW_DATABASE_ERROR(conn_->native_dbc_handle(), SQL_HANDLE_DBC);
			}
		}
	}

	void rollback() DBC_NOEXCEPT
	{
		if (committed_)
		{
			return;
		}

		tran_->rollback(true);
	}

	connection* get_connection()
	{
		return conn_;
	}

	const connection* get_connection() const
	{
		return conn_;
	}

private:
	transaction* tran_;
	connection* conn_;
	bool committed_;
};

transaction::transaction(connection* conn)
	: impl_(new transaction_impl(this, conn))
{
}

transaction::~transaction() DBC_NOEXCEPT
{
}

void transaction::commit()
{
	impl_->commit();
}

void transaction::rollback() DBC_NOEXCEPT
{
	impl_->rollback();
}

connection* transaction::get_connection()
{
	return impl_->get_connection();
}

const connection* transaction::get_connection() const
{
	return impl_->get_connection();
}

std::size_t transaction::ref_transaction()
{
	return get_connection()->ref_transaction();
}

std::size_t transaction::unref_transaction()
{
	return get_connection()->unref_transaction();
}

bool transaction::rollback() const
{
	return get_connection()->rollback();
}

void transaction::rollback(bool onoff)
{
	get_connection()->rollback(onoff);
}

} // dbc