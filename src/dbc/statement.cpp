#include "stdafx.h"

#include "statement.hpp"
#include "error.hpp"
#include "result.hpp"

#include <map>

namespace dbc
{

class statement::statement_impl
{
public:
	using results = std::vector<result::ptr>;

public:
	statement_impl(const statement_impl&) = delete;
	statement_impl& operator=(const statement_impl&) = delete;

	statement_impl(statement* stmt)
		: cstmt_(stmt)
		, stmt_(0)
		, open_(false)
		, conn_()
		, bind_len_or_null_()
	{
	}

	statement_impl(statement* stmt, connection* conn)
		: cstmt_(stmt)
		, stmt_(0)
		, open_(false)
		, conn_()
		, bind_len_or_null_()
	{
		open(conn);
	}

	statement_impl(statement* stmt, class connection* conn, const std::wstring& query, long timeout)
		: cstmt_(stmt)
		, stmt_(0)
		, open_(false)
		, conn_()
		, bind_len_or_null_()
	{
		prepare(conn, query, timeout);
	}

	~statement_impl() DBC_NOEXCEPT
	{
		if (open() && connected())
		{
			DBC_CALL(
				SQLCancel
				, stmt_);
			reset_parameters();
			DBC_CALL(
				SQLFreeHandle
				, SQL_HANDLE_STMT
				, stmt_);
		}

		results_.clear();
	}

	void open(connection* conn)
	{
		close();
		RETCODE rc;
		DBC_CALL_RC(
			SQLAllocHandle
			, rc
			, SQL_HANDLE_STMT
			, conn->native_dbc_handle()
			, &stmt_);

		open_ = success(rc);

		if (!open_)
		{
			DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
		}

		conn_ = conn;
	}

	bool open() const
	{
		return open_;
	}

	bool connected() const
	{
		return conn_ && conn_->connected();
	}

	const connection* get_connection() const
	{
		return conn_;
	}

	connection* get_connection()
	{
		return conn_;
	}

	void* native_statement_handle() const
	{
		return stmt_;
	}

	void close()
	{
		if (open() && connected())
		{
			RETCODE rc;

			DBC_CALL_RC(
				SQLCancel
				, rc
				, stmt_);

			if (!success(rc))
			{
				DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
			}

			reset_parameters();

			DBC_CALL_RC(
				SQLFreeHandle
				, rc
				, SQL_HANDLE_STMT
				, stmt_);

			if (!success(rc))
			{
				DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
			}
		}

		open_ = false;
		stmt_ = 0;
	}

	void cancel()
	{
		RETCODE rc;

		DBC_CALL_RC(
			SQLCancel
			, rc
			, stmt_);

		if (!success(rc))
		{
			DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
		}
	}

	void prepare(connection* conn, const std::wstring& query, long timeout)
	{
		open(conn);
		prepare(query, timeout);
	}

	void prepare(const std::wstring& query, long timeout)
	{
		if (!open())
		{
			throw programming_error("statement has no associated open connection");
		}

		query_changed_ = (query != query_);
		query_ = query;

		is_prepare_called_after_execute_ = true;

		RETCODE rc;

		DBC_CALL_RC(
			DBC_FUNC(SQLPrepare)
			, rc
			, stmt_
			, (DBC_SQLCHAR*)query.c_str()
			, (SQLINTEGER)query.size());

		if (!success(rc))
		{
			DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
		}

		this->timeout(timeout);
	}

	void timeout(long timeout)
	{
		RETCODE rc;

		DBC_CALL_RC(
			SQLSetStmtAttr
			, rc
			, stmt_
			, SQL_ATTR_QUERY_TIMEOUT
			, (SQLPOINTER)(std::intptr_t)timeout,
			0);

		// some drivers don't support timeout for statements,
		// so only raise the error if a non-default timeout was requested.
		if (!success(rc) && (timeout != 0))
		{
			DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
		}
	}

	void execute_direct(
		connection* conn
		, const std::wstring& query
		, long batch_operations
		, long timeout)
	{
#ifdef DBC_HANDLE_NODATA_BUG
		const RETCODE rc = just_execute_direct(conn, query, batch_operations, timeout);
		if (rc == SQL_NO_DATA)
			return result();
#else
		just_execute_direct(conn, query, batch_operations, timeout);
#endif

	}

	RETCODE just_execute_direct(
		connection* conn
		, const std::wstring& query
		, long batch_operations
		, long timeout
		, void* event_handle = NULL)
	{
		event_handle;

		open(conn);

#if defined(SQL_ATTR_ASYNC_STMT_EVENT) && defined(SQL_API_SQLCOMPLETEASYNC)
		if (event_handle != NULL)
			enable_async(event_handle);
#endif

		check_query_and_batch(query, batch_operations);

		current_result_set_ = -1;

		RETCODE rc;
		DBC_CALL_RC(
			SQLSetStmtAttr
			, rc
			, stmt_
			, SQL_ATTR_PARAMSET_SIZE
			, (SQLPOINTER)(std::intptr_t)batch_operations
			, 0);

		if (!success(rc))
		{
			DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
		}

		this->timeout(timeout);

		DBC_CALL_RC(
			DBC_FUNC(SQLExecDirect)
			, rc
			, stmt_
			, (DBC_SQLCHAR*)query.c_str()
			, SQL_NTS);

		if (!success(rc) && rc != SQL_NO_DATA && rc != SQL_STILL_EXECUTING)
		{
			DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
		}

		return rc;
	}

	void execute(long batch_operations, long timeout)
	{
#ifdef DBC_HANDLE_NODATA_BUG
		const RETCODE rc = just_execute(batch_operations, timeout, statement);
		if (rc == SQL_NO_DATA)
			return result();
#else
		just_execute(batch_operations, timeout);
#endif
		WISE_ASSERT(batch_operations == batch_count_);

		current_result_set_ = -1;
	}

	RETCODE just_execute(long batch_operations, long timeout)
	{
		RETCODE rc;

		if (open())
		{
			// The ODBC cursor must be closed before subsequent executions, as described
			// here http://msdn.microsoft.com/en-us/library/windows/desktop/ms713584%28v=vs.85%29.aspx
			//
			// However, we don't necessarily want to call SQLCloseCursor() because that
			// will cause an invalid cursor state in the case that no cursor is currently open.
			// A better solution is to use SQLFreeStmt() with the SQL_CLOSE option, which has
			// the same effect without the undesired limitations.
			
			// TODO: 실제 환경과 유사한 조건에서 장시간 테스트 할 필요 있음

			DBC_CALL_RC(
				SQLFreeStmt
				, rc
				, stmt_
				, SQL_CLOSE);

			if (!success(rc))
			{
				DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
			}
		}

		if (query_changed_)
		{
			if (!is_prepare_called_after_execute_)
			{
				query_changed_ = false;
			}
		}

		batch_count_changed_ = (batch_operations != batch_count_);
		batch_count_ = batch_operations;

		DBC_CALL_RC(
			SQLSetStmtAttr
			, rc
			, stmt_
			, SQL_ATTR_PARAMSET_SIZE
			, (SQLPOINTER)(std::intptr_t)batch_operations
			, 0);

		if (!success(rc) && rc != SQL_NO_DATA)
		{
			DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
		}

		this->timeout(timeout);

		DBC_CALL_RC(
			SQLExecute
			, rc
			, stmt_);

		if (!success(rc) && rc != SQL_NO_DATA)
		{
			DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
		}

		is_prepare_called_after_execute_ = false;

		return rc;
	}

	result& next_result()
	{
		// -1 일 경우 최초 결과로 result를 만들어서 확인한다.
		if (current_result_set_ == -1)
		{
			if (columns() < 1)
			{
				return empty_result_;
			}
		}
		else
		{
			WISE_ASSERT(current_result_set_ >= 0);

			RETCODE rc;

			DBC_CALL_RC(
				SQLMoreResults
				, rc
				, stmt_);

			if (rc == SQL_NO_DATA)
			{
				return empty_result_;
			}

			if (!success(rc))
			{
				DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
			}
		}

		current_result_set_++;

		if (results_.size() <= current_result_set_)
		{
			results_.push_back(wise_shared<result>());
		}

		results_[current_result_set_]->prepare_fetch(
			cstmt_,
			batch_count_,
			batch_count_changed_ || query_changed_,
			current_result_set_
		);

		return *results_[current_result_set_].get();
	}

	long affected_rows() const
	{
		SQLLEN rows;
		RETCODE rc;
		DBC_CALL_RC(
			SQLRowCount
			, rc
			, stmt_
			, &rows);
		if (!success(rc))
			DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
		WISE_ASSERT(rows <= static_cast<SQLLEN>(std::numeric_limits<long>::max()));
		return static_cast<long>(rows);
	}

	short columns() const
	{
		SQLSMALLINT cols;
		RETCODE rc;
		DBC_CALL_RC(
			SQLNumResultCols
			, rc
			, stmt_
			, &cols);

		if (!success(rc))
		{
			DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
		}

		return cols;
	}

	void reset_parameters() DBC_NOEXCEPT
	{
		DBC_CALL(
			SQLFreeStmt
			, stmt_
			, SQL_RESET_PARAMS);
	}

	unsigned long parameter_size(short param) const
	{
		RETCODE rc;
		SQLSMALLINT data_type;
		SQLSMALLINT nullable;
		SQLULEN parameter_size;

		DBC_CALL_RC(
			SQLDescribeParam
			, rc
			, stmt_
			, param + 1
			, &data_type
			, &parameter_size
			, 0
			, &nullable);

		if (!success(rc))
		{
			DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
		}

		WISE_ASSERT(parameter_size <= static_cast<SQLULEN>(std::numeric_limits<unsigned long>::max()));

		return static_cast<unsigned long>(parameter_size);
	}

	static SQLSMALLINT param_type_from_direction(param_direction direction)
	{
		switch (direction)
		{
		case PARAM_IN:
			return SQL_PARAM_INPUT;
			break;
		case PARAM_OUT:
			return SQL_PARAM_OUTPUT;
			break;
		case PARAM_INOUT:
			return SQL_PARAM_INPUT_OUTPUT;
			break;
		case PARAM_RETURN:
			return SQL_PARAM_OUTPUT;
			break;
		default:
			WISE_ASSERT(false);
			throw programming_error("unrecognized param_direction value");
		}
	}

	// initializes bind_len_or_null_ and gets information for bind
	void prepare_bind(
		short param
		, std::size_t elements
		, param_direction direction
		, SQLSMALLINT& data_type
		, SQLSMALLINT& param_type
		, SQLULEN& parameter_size
		, SQLSMALLINT& scale)
	{
		RETCODE rc;
		SQLSMALLINT nullable;
		DBC_CALL_RC(
			SQLDescribeParam
			, rc
			, stmt_
			, param + 1
			, &data_type
			, &parameter_size
			, &scale
			, &nullable);

		if (!success(rc))
			DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);

		param_type = param_type_from_direction(direction);

		if (!bind_len_or_null_.count(param))
		{
			bind_len_or_null_[param] = std::vector<null_type>();
		}

		std::vector<null_type>().swap(bind_len_or_null_[param]);

		// ODBC weirdness: this must be at least 8 elements in size
		const std::size_t indicator_size = elements > 8 ? elements : 8;

		bind_len_or_null_[param].reserve(indicator_size);
		bind_len_or_null_[param].assign(indicator_size, SQL_NULL_DATA);
	}

	// calls actual ODBC bind parameter function
	template<class T>
	void bind_parameter(
		short param
		, const T* data
		, std::size_t /*elements*/
		, SQLSMALLINT data_type
		, SQLSMALLINT param_type
		, SQLULEN parameter_size
		, SQLSMALLINT scale)
	{
		RETCODE rc;

		SQLSMALLINT value_type = sql_ctype<T>::value;

		DBC_CALL_RC(
			SQLBindParameter
			, rc
			, stmt_ // handle
			, param + 1 // parameter number
			, param_type // input or output type
			, value_type // value type
			, data_type // parameter type
			, parameter_size // column size ignored for many types, but needed for strings
			, scale // decimal digits
			, (SQLPOINTER)data // parameter value
			, parameter_size // buffer length
			, bind_len_or_null_[param].data());

		if (!success(rc))
		{
			DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
		}
	}

	// handles a single value (possibly a single string value), or multiple non-string values
	template<class T>
	void bind(short param, const T* values, std::size_t elements, param_direction direction);

	void bind_uint8(short param, uint8_t& value, param_direction direction = PARAM_IN)
	{
		SQLSMALLINT data_type;
		SQLSMALLINT param_type;
		SQLULEN parameter_size;
		SQLSMALLINT scale;

		prepare_bind(param, 1, direction, data_type, param_type, parameter_size, scale);

		bind_len_or_null_[param][0] = parameter_size;

		RETCODE rc;

		DBC_CALL_RC(
			SQLBindParameter
			, rc
			, stmt_ // handle
			, param + 1 // parameter number
			, param_type // input or output type
			, SQL_C_UTINYINT
			, data_type // parameter type
			, parameter_size // column size ignored for many types, but needed for strings
			, scale // decimal digits
			, (SQLPOINTER)&value // parameter value
			, parameter_size // buffer length
			, NULL);

		if (!success(rc))
			DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
	}

	void bind_int8(short param, int8_t& value, param_direction direction = PARAM_IN)
	{
		SQLSMALLINT data_type;
		SQLSMALLINT param_type;
		SQLULEN parameter_size;
		SQLSMALLINT scale;

		prepare_bind(param, 1, direction, data_type, param_type, parameter_size, scale);

		bind_len_or_null_[param][0] = parameter_size;

		RETCODE rc;

		DBC_CALL_RC(
			SQLBindParameter
			, rc
			, stmt_ // handle
			, param + 1 // parameter number
			, param_type // input or output type
			, SQL_C_TINYINT
			, data_type // parameter type
			, parameter_size // column size ignored for many types, but needed for strings
			, scale // decimal digits
			, (SQLPOINTER)&value // parameter value
			, parameter_size // buffer length
			, NULL);

		if (!success(rc))
			DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
	}

	void bind_bool(short param, bool& value, param_direction direction = PARAM_IN)
	{
		SQLSMALLINT data_type;
		SQLSMALLINT param_type;
		SQLULEN parameter_size;
		SQLSMALLINT scale;

		prepare_bind(param, 1, direction, data_type, param_type, parameter_size, scale);

		bind_len_or_null_[param][0] = parameter_size;

		RETCODE rc;

		DBC_CALL_RC(
			SQLBindParameter
			, rc
			, stmt_ // handle
			, param + 1 // parameter number
			, param_type // input or output type
			, sql_ctype<bool>::value // value type
			, data_type // parameter type
			, parameter_size // column size ignored for many types, but needed for strings
			, scale // decimal digits
			, (SQLPOINTER)&value // parameter value
			, parameter_size // buffer length
			, NULL);

		if (!success(rc))
			DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
	}

	// handles multiple string values
	void bind_strings(
		short param
		, const std::wstring::value_type* values
		, std::size_t /*length*/
		, std::size_t elements
		, param_direction direction)
	{
		bind(param, values, elements, direction);
	}

	// handles multiple string values
	void bind_strings(
		short param
		, const std::string::value_type* values
		, std::size_t /*length*/
		, std::size_t elements
		, param_direction direction)
	{
		bind(param, values, elements, direction);
	}

	// handles multiple null values
	void bind_null(short param, std::size_t elements)
	{
		SQLSMALLINT data_type;
		SQLSMALLINT param_type;
		SQLULEN parameter_size;
		SQLSMALLINT scale;
		prepare_bind(param, elements, PARAM_IN, data_type, param_type, parameter_size, scale);

		RETCODE rc;
		DBC_CALL_RC(
			SQLBindParameter
			, rc
			, stmt_
			, param + 1
			, param_type
			, SQL_C_CHAR
			, data_type
			, parameter_size // column size ignored for many types, but needed for strings
			, 0
			, (SQLPOINTER)0 // null value
			, 0 // parameter_size
			, bind_len_or_null_[param].data());
		if (!success(rc))
			DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
	}

	// comparator for null sentry values
	template<class T>
	bool equals(const T& lhs, const T& rhs)
	{
		return lhs == rhs;
	}

	// handles multiple non-string values with a null sentry
	template<class T>
	void bind(
		short param
		, const T* values
		, std::size_t elements
		, const bool* nulls
		, const T* null_sentry
		, param_direction direction);

	// handles multiple string values
	void bind_strings(
		short param
		, const std::wstring::value_type* values
		, std::size_t length
		, std::size_t elements
		, const bool* nulls
		, const std::wstring::value_type* null_sentry
		, param_direction direction);

	// handles multiple string values
	void bind_strings(
		short param
		, const std::string::value_type* values
		, std::size_t length
		, std::size_t elements
		, const bool* nulls
		, const std::string::value_type* null_sentry
		, param_direction direction);

	bool check_query_and_batch(const std::wstring& query, long batch_operations)
	{
		query_changed_ = (query != query_);
		query_ = query;

		batch_count_changed_ = (batch_operations != batch_count_);
		batch_count_ = batch_operations;

		return (query_changed_ || batch_count_changed_);
	}

	results& get_reslut_sets()
	{
		return results_;
	}

private:

	HSTMT stmt_;
	statement* cstmt_ = nullptr;
	bool open_;
	connection* conn_ = nullptr;
	results results_;
	long current_result_set_ = -1;
	std::wstring query_;
	bool query_changed_ = true;
	long batch_count_ = 1;
	bool batch_count_changed_ = true;
	std::map<short, std::vector<null_type> > bind_len_or_null_;
	result empty_result_;
	bool is_prepare_called_after_execute_ = false;
};

// Supports code like: query.bind(0, string.c_str())
// In this case, we need to pass NULL to the final parameter of SQLBindParameter().
template<>
void statement::statement_impl::bind_parameter<std::wstring::value_type>(
	short param
	, const std::wstring::value_type* data
	, std::size_t elements
	, SQLSMALLINT data_type
	, SQLSMALLINT param_type
	, SQLULEN parameter_size
	, SQLSMALLINT scale)
{
	RETCODE rc;
	DBC_CALL_RC(
		SQLBindParameter
		, rc
		, stmt_ // handle
		, param + 1 // parameter number
		, param_type // input or output type
		, sql_ctype<std::wstring::value_type>::value // value type
		, data_type // parameter type
		, parameter_size // column size ignored for many types, but needed for strings
		, scale // decimal digits
		, (SQLPOINTER)data // parameter value
		, parameter_size // buffer length
		, (elements <= 1 ? NULL : bind_len_or_null_[param].data()));

	if (!success(rc))
		DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
}

template<>
void statement::statement_impl::bind_parameter<std::string::value_type>(
	short param
	, const std::string::value_type* data
	, std::size_t elements
	, SQLSMALLINT data_type
	, SQLSMALLINT param_type
	, SQLULEN parameter_size
	, SQLSMALLINT scale)
{
	RETCODE rc;
	DBC_CALL_RC(
		SQLBindParameter
		, rc
		, stmt_ // handle
		, param + 1 // parameter number
		, param_type // input or output type
		, sql_ctype<std::string::value_type>::value // value type
		, data_type // parameter type
		, parameter_size // column size ignored for many types, but needed for strings
		, scale // decimal digits
		, (SQLPOINTER)data // parameter value
		, parameter_size // buffer length
		, (elements <= 1 ? NULL : bind_len_or_null_[param].data()));

	if (!success(rc))
		DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
}

template<>
void statement::statement_impl::bind_parameter<dbc::timestamp>(
	short param
	, const dbc::timestamp* data
	, std::size_t elements
	, SQLSMALLINT 
	, SQLSMALLINT param_type
	, SQLULEN parameter_size
	, SQLSMALLINT scale)
{
	RETCODE rc;
	DBC_CALL_RC(
		SQLBindParameter
		, rc
		, stmt_ // handle
		, param + 1 // parameter number
		, param_type // input or output type
		, SQL_C_TYPE_TIMESTAMP
		, SQL_TYPE_TIMESTAMP
		, parameter_size // column size ignored for many types, but needed for strings
		, scale // decimal digits
		, (SQLPOINTER)data // parameter value
		, SQL_TIMESTAMP_LEN * elements // buffer length
		, (elements <= 1 ? NULL : bind_len_or_null_[param].data()));

	if (!success(rc))
		DBC_THROW_DATABASE_ERROR(stmt_, SQL_HANDLE_STMT);
}

template<class T>
void statement::statement_impl::bind(
	short param
	, const T* values
	, std::size_t elements
	, param_direction direction)
{
	SQLSMALLINT data_type;
	SQLSMALLINT param_type;
	SQLULEN parameter_size;
	SQLSMALLINT scale;
	prepare_bind(param, elements, direction, data_type, param_type, parameter_size, scale);

	for (std::size_t i = 0; i < elements; ++i)
		bind_len_or_null_[param][i] = parameter_size;

	// param_type이 방향을 갖고 있다. 
	bind_parameter(param, values, elements, data_type, param_type, parameter_size, scale);
}

template<class T>
void statement::statement_impl::bind(
	short param
	, const T* values
	, std::size_t elements
	, const bool* nulls
	, const T* null_sentry
	, param_direction direction)
{
	SQLSMALLINT data_type;
	SQLSMALLINT param_type;
	SQLULEN parameter_size;
	SQLSMALLINT scale;
	prepare_bind(param, elements, direction, data_type, param_type, parameter_size, scale);

	for (std::size_t i = 0; i < elements; ++i)
		if ((null_sentry && !equals(values[i], *null_sentry)) || (nulls && !nulls[i]))
			bind_len_or_null_[param][i] = parameter_size;

	bind_parameter(param, values, elements, data_type, param_type, parameter_size, scale);
}

void statement::statement_impl::bind_strings(
	short param
	, const std::wstring::value_type* values
	, std::size_t length
	, std::size_t elements
	, const bool* nulls
	, const std::wstring::value_type* null_sentry
	, param_direction direction)
{
	SQLSMALLINT data_type;
	SQLSMALLINT param_type;
	SQLULEN parameter_size;
	SQLSMALLINT scale;
	prepare_bind(param, elements, direction, data_type, param_type, parameter_size, scale);

	if (null_sentry)
	{
		for (std::size_t i = 0; i < elements; ++i)
		{
			const std::wstring s_lhs(values + i * length, values + (i + 1) * length);
			const std::wstring s_rhs(null_sentry);
#if DBC_USE_UNICODE
			std::string narrow_lhs;
			narrow_lhs.reserve(s_lhs.size());
			convert(s_lhs, narrow_lhs);
			std::string narrow_rhs;
			narrow_rhs.reserve(s_rhs.size());
			convert(s_rhs, narrow_lhs);
			if (std::strncmp(narrow_lhs.c_str(), narrow_rhs.c_str(), length))
				bind_len_or_null_[param][i] = parameter_size;
#else
			if (std::strncmp(s_lhs.c_str(), s_rhs.c_str(), length))
				bind_len_or_null_[param][i] = parameter_size;
#endif
		}
	}
	else if (nulls)
	{
		for (std::size_t i = 0; i < elements; ++i)
		{
			if (!nulls[i])
				bind_len_or_null_[param][i] = SQL_NTS; // null terminated
		}
	}

	bind_parameter(param, values, elements, data_type, param_type, parameter_size, scale);
}

void statement::statement_impl::bind_strings(
	short param
	, const std::string::value_type* values
	, std::size_t length
	, std::size_t elements
	, const bool* nulls
	, const std::string::value_type* null_sentry
	, param_direction direction)
{
	SQLSMALLINT data_type;
	SQLSMALLINT param_type;
	SQLULEN parameter_size;
	SQLSMALLINT scale;
	prepare_bind(param, elements, direction, data_type, param_type, parameter_size, scale);

	if (null_sentry)
	{
		for (std::size_t i = 0; i < elements; ++i)
		{
			const std::string s_lhs(values + i * length, values + (i + 1) * length);
			const std::string s_rhs(null_sentry);

			if (std::strncmp(s_lhs.c_str(), s_rhs.c_str(), length))
				bind_len_or_null_[param][i] = parameter_size;
		}
	}
	else if (nulls)
	{
		for (std::size_t i = 0; i < elements; ++i)
		{
			if (!nulls[i])
				bind_len_or_null_[param][i] = SQL_NTS; // null terminated
		}
	}

	bind_parameter(param, values, elements, data_type, param_type, parameter_size, scale);
}


template<>
bool statement::statement_impl::equals(const date& lhs, const date& rhs)
{
	return lhs.year == rhs.year
		&& lhs.month == rhs.month
		&& lhs.day == rhs.day;
}

template<>
bool statement::statement_impl::equals(const timestamp& lhs, const timestamp& rhs)
{
	return lhs.year == rhs.year
		&& lhs.month == rhs.month
		&& lhs.day == rhs.day
		&& lhs.hour == rhs.hour
		&& lhs.min == rhs.min
		&& lhs.sec == rhs.sec
		&& lhs.fract == rhs.fract;
}

statement::statement()
	: impl_(new statement_impl(this))
{
}

statement::statement(connection* conn)
	: impl_(new statement_impl(this, conn))
{
}

statement::statement(connection* conn, const std::wstring& query, long timeout)
	: impl_(new statement_impl(this, conn, query, timeout))
{
}

statement::~statement() DBC_NOEXCEPT
{
}

void statement::open(connection* conn)
{
	impl_->open(conn);
}

bool statement::open() const
{
	return impl_->open();
}

bool statement::connected() const
{
	return impl_->connected();
}

const connection* statement::get_connection() const
{
	return impl_->get_connection();
}

connection* statement::get_connection()
{
	return impl_->get_connection();
}

void* statement::native_statement_handle() const
{
	return impl_->native_statement_handle();
}

void statement::close()
{
	impl_->close();
}

void statement::cancel()
{
	impl_->cancel();
}

void statement::prepare(connection* conn, const std::wstring& query, long timeout)
{
	impl_->prepare(conn, query, timeout);
}

void statement::prepare(const std::wstring& query, long timeout)
{
	impl_->prepare(query, timeout);
}

void statement::timeout(long timeout)
{
	impl_->timeout(timeout);
}

void statement::execute_direct(
	  connection* conn
	, const std::wstring& query
	, long batch_operations
	, long timeout)
{
	impl_->execute_direct(conn, query, batch_operations, timeout);
}

void statement::just_execute_direct(
	  connection* conn
	, const std::wstring& query
	, long batch_operations
	, long timeout)
{
	impl_->just_execute_direct(conn, query, batch_operations, timeout);
}

void statement::execute(long batch_operations, long timeout)
{
	impl_->execute(batch_operations, timeout);
}

void statement::just_execute(long batch_operations, long timeout)
{
	impl_->just_execute(batch_operations, timeout);
}

result& statement::next_result()
{
	return impl_->next_result();
}

long statement::affected_rows() const
{
	return impl_->affected_rows();
}

short statement::columns() const
{
	return impl_->columns();
}

void statement::reset_parameters() DBC_NOEXCEPT
{
	impl_->reset_parameters();
}

unsigned long statement::parameter_size(short param) const
{
	return impl_->parameter_size(param);
}


template<class T>
void statement::bind(short param, const T* value, param_direction direction)
{
	impl_->bind(param, value, 1, direction);
}

template<class T>
void statement::bind(short param, const T* values, std::size_t elements, param_direction direction)
{
	impl_->bind(param, values, elements, direction);
}

template<class T>
void statement::bind(
	short param
	, const T* values
	, std::size_t elements
	, const T* null_sentry
	, param_direction direction)
{
	impl_->bind(param, values, elements, 0, null_sentry, direction);
}

template<class T>
void statement::bind(
	short param
	, const T* values
	, std::size_t elements
	, const bool* nulls
	, param_direction direction)
{
	impl_->bind(param, values, elements, nulls, (T*)0, direction);
}

void statement::bind_uint8(short param, uint8_t& value, param_direction direction)
{
	impl_->bind_uint8(param, value, direction);
}

void statement::bind_int8(short param, int8_t& value, param_direction direction)
{
	impl_->bind_int8(param, value, direction);
}

void statement::bind_bool(short param, bool& value, param_direction direction)
{
	impl_->bind_bool(param, value, direction);
}

void statement::bind_strings(
	short param
	, const std::wstring::value_type* values
	, std::size_t length
	, std::size_t elements
	, param_direction direction)
{
	impl_->bind_strings(param, values, length, elements, direction);
}

void statement::bind_strings(
	short param
	, const std::wstring::value_type* values
	, std::size_t length
	, std::size_t elements
	, const std::wstring::value_type* null_sentry
	, param_direction direction)
{
	impl_->bind_strings(param, values, length, elements, (bool*)0, null_sentry, direction);
}

void statement::bind_strings(
	short param
	, const std::wstring::value_type* values
	, std::size_t length
	, std::size_t elements
	, const bool* nulls
	, param_direction direction)
{
	impl_->bind_strings(
		param
		, values
		, length
		, elements
		, nulls
		, (std::wstring::value_type*)0
		, direction);
}

void statement::bind_strings(
	short param
	, const std::string::value_type* values
	, std::size_t length
	, std::size_t elements
	, param_direction direction)
{
	impl_->bind_strings(param, values, length, elements, direction);
}

void statement::bind_strings(
	short param
	, const std::string::value_type* values
	, std::size_t length
	, std::size_t elements
	, const std::string::value_type* null_sentry
	, param_direction direction)
{
	impl_->bind_strings(param, values, length, elements, (bool*)0, null_sentry, direction);
}

void statement::bind_strings(
	short param
	, const std::string::value_type* values
	, std::size_t length
	, std::size_t elements
	, const bool* nulls
	, param_direction direction)
{
	impl_->bind_strings(
		param
		, values
		, length
		, elements
		, nulls
		, (std::string::value_type*)0
		, direction);
}

void statement::bind_null(short param, std::size_t elements)
{
	impl_->bind_null(param, elements);
}

std::size_t statement::get_result_set_count() const
{
	return impl_->get_reslut_sets().size();
}


// We need to instantiate each form of bind() for each of our supported data types.
#define DBC_INSTANTIATE_BINDS(type)                                                                                 \
    template void statement::bind(short, const type*, param_direction); /* 1-ary */                                     \
    template void statement::bind(short, const type*, std::size_t, param_direction); /* n-ary */                        \
    template void statement::bind(short, const type*, std::size_t, const type*, param_direction); /* n-ary, sentry */   \
    template void statement::bind(short, const type*, std::size_t, const bool*, param_direction) /* n-ary, flags */     \
    /**/

// The following are the only supported instantiations of statement::bind().
DBC_INSTANTIATE_BINDS(std::wstring::value_type);
DBC_INSTANTIATE_BINDS(std::string::value_type);
DBC_INSTANTIATE_BINDS(short);
DBC_INSTANTIATE_BINDS(unsigned short);
DBC_INSTANTIATE_BINDS(int32_t);
DBC_INSTANTIATE_BINDS(uint32_t);
DBC_INSTANTIATE_BINDS(int64_t);
DBC_INSTANTIATE_BINDS(uint64_t);
DBC_INSTANTIATE_BINDS(float);
DBC_INSTANTIATE_BINDS(double);
DBC_INSTANTIATE_BINDS(date);
DBC_INSTANTIATE_BINDS(timestamp);

#undef DBC_INSTANTIATE_BINDS

} // dbc