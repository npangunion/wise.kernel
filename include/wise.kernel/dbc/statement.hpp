#pragma once 

#include "common.hpp"
#include "connection.hpp"
#include "result.hpp"

#include <functional>
#include <memory>

namespace dbc
{

//! \brief Represents a statement on the database.
class statement
{
public:
	//! \brief Provides support for retrieving output/return parameters.
	//! \see binding
	enum param_direction
	{
		  PARAM_IN //!< Binding an input parameter.
		, PARAM_OUT //!< Binding an output parameter.
		, PARAM_INOUT //!< Binding an input/output parameter.
		, PARAM_RETURN //!< Binding a return parameter.
	};

	using ptr = std::shared_ptr<statement>;

public:
	//! \brief Creates a new un-prepared statement.
	//! \see execute(), just_execute(), execute_direct(), just_execute_direct(), open(), prepare()
	statement();

	//! \brief Constructs a statement object and associates it to the given connection.
	//! \param conn The connection to use.
	//! \see open(), prepare()
	explicit statement(connection* conn);

	//! \brief Constructs and prepares a statement using the given connection and query.
	//! \param conn The connection to use.
	//! \param query The SQL query statement.
	//! \param timeout The number in seconds before query timeout. Default is 0 indicating no timeout.
	//! \see execute(), just_execute(), execute_direct(), just_execute_direct(), open(), prepare()
	statement(connection* conn, const std::wstring& query, long timeout = 0);

	// statement is shared. no move around
	statement(const statement& rhs) = delete;
	statement(statement&& rhs) = delete;
	statement& operator=(statement rhs) = delete;

	//! \brief Closes the statement.
	//! \see close()
	~statement() DBC_NOEXCEPT;

	//! \brief Creates a statement for the given connection.
	//! \param conn The connection where the statement will be executed.
	//! \throws database_error
	void open(connection* conn);

	//! \brief Returns true if connection is open.
	bool open() const;

	//! \brief Returns true if connected to the database.
	bool connected() const;

	//! \brief Returns the associated connection object if any.
	connection* get_connection();

	//! \brief Returns the associated connection object if any.
	const connection* get_connection() const;

	//! \brief Returns the native ODBC statement handle.
	void* native_statement_handle() const;

	//! \brief Closes the statement and frees all associated resources.
	void close();

	//! \brief Cancels execution of the statement.
	//! \throws database_error
	void cancel();

	//! \brief Opens and prepares the given statement to execute on the given connection.
	//! \param conn The connection where the statement will be executed.
	//! \param query The SQL query that will be executed.
	//! \param timeout The number in seconds before query timeout. Default is 0 indicating no timeout.
	//! \see open()
	//! \throws database_error
	void prepare(connection* conn, const std::wstring& query, long timeout = 0);

	//! \brief Prepares the given statement to execute its associated connection.
	//! If the statement is not open throws programming_error.
	//! \param query The SQL query that will be executed.
	//! \param timeout The number in seconds before query timeout. Default is 0 indicating no timeout.
	//! \see open()
	//! \throws database_error, programming_error
	void prepare(const std::wstring& query, long timeout = 0);

	//! \brief Sets the number in seconds before query timeout. Default is 0 indicating no timeout.
	//! \throws database_error
	void timeout(long timeout = 0);

	//! \brief Immediately opens, prepares, and executes the given query directly on the given connection.
	//! \param conn The connection where the statement will be executed.
	//! \param query The SQL query that will be executed.
	//! \param batch_operations Numbers of rows to fetch per rowset, or the number of batch parameters to process.
	//! \param timeout The number in seconds before query timeout. Default is 0 indicating no timeout.
	//! \return A result set object.
	//! \attention You will want to use transactions if you are doing batch operations because it will prevent auto commits from occurring after each individual operation is executed.
	//! \see open(), prepare(), execute(), result, transaction
	void execute_direct(connection* conn, const std::wstring& query, long batch_operations = 1, long timeout = 0);

	//! \brief Execute the previously prepared query now without constructing result object.
	//! \param conn The connection where the statement will be executed.
	//! \param query The SQL query that will be executed.
	//! \param batch_operations Numbers of rows to fetch per rowset, or the number of batch parameters to process.
	//! \param timeout The number in seconds before query timeout. Default is 0 indicating no timeout.
	//! \throws database_error
	//! \return A result set object.
	//! \attention You will want to use transactions if you are doing batch operations because it will prevent auto commits from occurring after each individual operation is executed.
	//! \see open(), prepare(), execute(), execute_direct(), result, transaction
	void just_execute_direct(connection* conn, const std::wstring& query, long batch_operations = 1, long timeout = 0);

	//! \brief Execute the previously prepared query now.
	//! \param batch_operations Numbers of rows to fetch per rowset, or the number of batch parameters to process.
	//! \param timeout The number in seconds before query timeout. Default is 0 indicating no timeout.
	//! \throws database_error
	//! \return A result set object.
	//! \attention You will want to use transactions if you are doing batch operations because it will prevent auto commits from occurring after each individual operation is executed.
	//! \see open(), prepare(), result, transaction
	void execute(long batch_operations = 1, long timeout = 0);

	//! \brief Execute the previously prepared query now without constructing result object.
	//! \param batch_operations Numbers of rows to fetch per rowset, or the number of batch parameters to process.
	//! \param timeout The number in seconds before query timeout. Default is 0 indicating no timeout.
	//! \throws database_error
	//! \return A result set object.
	//! \attention You will want to use transactions if you are doing batch operations because it will prevent auto commits from occurring after each individual operation is executed.
	//! \see open(), prepare(), execute(), result, transaction
	void just_execute(long batch_operations = 1, long timeout = 0);

	//! Returns the next result, for example when stored procedure returns multiple result sets.
	result& next_result();

	//! \brief Returns the number of rows affected by the request or -1 if the number of affected rows is not available.
	//! \throws database_error
	long affected_rows() const;

	//! \brief Returns the number of columns in a result set.
	//! \throws database_error
	short columns() const;

	//! \brief Resets all currently bound parameters.
	void reset_parameters() DBC_NOEXCEPT;

	//! \brief Returns the parameter size for the indicated parameter placeholder within a prepared statement.
	unsigned long parameter_size(short param) const;

	//! \addtogroup binding Binding parameters
	//! \brief These functions are used to bind values to ODBC parameters.
	//!
	//! @{

	//! \brief Binds the given value to the given parameter placeholder number in the prepared statement.
	//!
	//! If your prepared SQL query has any ? placeholders, this is how you bind values to them.
	//! Placeholder numbers count from left to right and are 0-indexed.
	//!
	//! It is NOT possible to use these functions for bulk operations as number of elements is not specified here.
	//!
	//! \param param Placeholder position.
	//! \param value Value to substitute into placeholder.
	//! \param direction ODBC parameter direction.
	//! \throws database_error
	template<class T>
	void bind(short param, const T* value, param_direction direction = PARAM_IN);

	//! \addtogroup bind_multi Binding multiple non-string values
	//! \brief Binds the given values to the given parameter placeholder number in the prepared statement.
	//!
	//! If your prepared SQL query has any ? placeholders, this is how you bind values to them.
	//! Placeholder numbers count from left to right and are 0-indexed.
	//!
	//! It is possible to use these functions for bulk operations.
	//!
	//! \param param Placeholder position.
	//! \param values Values to substitute into placeholder.
	//! \param elements The number of elements being bound.
	//! \param null_sentry Value which should represent a null value.
	//! \param nulls Flags for values that should be set to a null value.
	//! \param param_direciton ODBC parameter direction.
	//! \throws database_error
	//!
	//! @{

	//! \brief Binds multiple values.
	//! \see bind_multi
	template<class T>
	void bind(short param, const T* values, std::size_t elements, param_direction direction = PARAM_IN);

	//! \brief Binds multiple values.
	//! \see bind_multi
	template<class T>
	void bind(short param, const T* values, std::size_t elements, const T* null_sentry, param_direction direction = PARAM_IN);

	//! \brief Binds multiple values.
	//! \see bind_multi
	template<class T>
	void bind(short param, const T* values, std::size_t elements, const bool* nulls, param_direction direction = PARAM_IN);

	//! @}

	void bind_int8(short param, int8_t& value, param_direction direction = PARAM_IN);
	void bind_uint8(short param, uint8_t& value, param_direction direction = PARAM_IN);
	void bind_bool(short param, bool& value, param_direction direction = PARAM_IN);

	//! \addtogroup bind_strings Binding multiple string values
	//! \brief Binds the given string values to the given parameter placeholder number in the prepared statement.
	//!
	//! If your prepared SQL query has any ? placeholders, this is how you bind values to them.
	//! Placeholder numbers count from left to right and are 0-indexed.
	//!
	//! It is possible to use these functions for bulk operations.
	//!
	//! \param param Placeholder position.
	//! \param values Values to substitute into placeholder.
	//! \param length Maximum length of string elements.
	//! \param elements The number of elements being bound. Otherwise the value N is taken as the number of elements.
	//! \param null_sentry Value which should represent a null value.
	//! \param nulls Flags for values that should be set to a null value.
	//! \param param_direciton ODBC parameter direction.
	//! \throws database_error
	//!
	//! @{

	//! \brief Binds multiple string values.
	//! \see bind_strings
	void bind_strings(
		short param
		, const std::wstring::value_type* values
		, std::size_t length
		, std::size_t elements
		, param_direction direction = PARAM_IN);

	//! \brief Binds multiple string values.
	//! \see bind_strings
	template<std::size_t N, std::size_t M>
	void bind_strings(
		short param
		, const std::wstring::value_type(&values)[N][M]
		, param_direction direction = PARAM_IN)
	{
		bind_strings(
			param
			, reinterpret_cast<const std::wstring::value_type*>(values)
			, M
			, N
			, direction);
	}

	//! \brief Binds multiple string values.
	//! \see bind_strings
	void bind_strings(
		short param
		, const std::wstring::value_type* values
		, std::size_t length
		, std::size_t elements
		, const std::wstring::value_type* null_sentry
		, param_direction direction = PARAM_IN);

	//! \brief Binds multiple string values.
	//! \see bind_strings
	template<std::size_t N, std::size_t M>
	void bind_strings(
		short param
		, const std::wstring::value_type(&values)[N][M]
		, const std::wstring::value_type* null_sentry
		, param_direction direction = PARAM_IN)
	{
		bind_strings(
			param
			, reinterpret_cast<const std::wstring::value_type*>(values)
			, M
			, N
			, null_sentry
			, direction);
	}

	//! \brief Binds multiple string values.
	//! \see bind_strings
	void bind_strings(
		short param
		, const std::wstring::value_type* values
		, std::size_t length
		, std::size_t elements
		, const bool* nulls
		, param_direction direction = PARAM_IN);

	//! \brief Binds multiple string values.
	//! \see bind_strings
	template<std::size_t N, std::size_t M>
	void bind_strings(
		short param
		, const std::wstring::value_type(&values)[N][M]
		, const bool* nulls
		, param_direction direction = PARAM_IN)
	{
		bind_strings(
			param
			, reinterpret_cast<const std::wstring::value_type*>(values)
			, M
			, N
			, nulls
			, direction);
	}


	//! \brief Binds multiple string values.
	//! \see bind_strings
	void bind_strings(
		short param
		, const std::string::value_type* values
		, std::size_t length
		, std::size_t elements
		, param_direction direction = PARAM_IN);

	//! \brief Binds multiple string values.
	//! \see bind_strings
	template<std::size_t N, std::size_t M>
	void bind_strings(
		short param
		, const std::string::value_type(&values)[N][M]
		, param_direction direction = PARAM_IN)
	{
		bind_strings(
			param
			, reinterpret_cast<const std::string::value_type*>(values)
			, M
			, N
			, direction);
	}

	//! \brief Binds multiple string values.
	//! \see bind_strings
	void bind_strings(
		short param
		, const std::string::value_type* values
		, std::size_t length
		, std::size_t elements
		, const std::string::value_type* null_sentry
		, param_direction direction = PARAM_IN);

	//! \brief Binds multiple string values.
	//! \see bind_strings
	template<std::size_t N, std::size_t M>
	void bind_strings(
		short param
		, const std::string::value_type(&values)[N][M]
		, const std::string::value_type* null_sentry
		, param_direction direction = PARAM_IN)
	{
		bind_strings(
			param
			, reinterpret_cast<const std::string::value_type*>(values)
			, M
			, N
			, null_sentry
			, direction);
	}

	//! \brief Binds multiple string values.
	//! \see bind_strings
	void bind_strings(
		short param
		, const std::string::value_type* values
		, std::size_t length
		, std::size_t elements
		, const bool* nulls
		, param_direction direction = PARAM_IN);

	//! \brief Binds multiple string values.
	//! \see bind_strings
	template<std::size_t N, std::size_t M>
	void bind_strings(
		short param
		, const std::string::value_type(&values)[N][M]
		, const bool* nulls
		, param_direction direction = PARAM_IN)
	{
		bind_strings(
			param
			, reinterpret_cast<const std::string::value_type*>(values)
			, M
			, N
			, nulls
			, direction);
	}

	//! @}

	//! \brief Binds null values to the given parameter placeholder number in the prepared statement.
	//!
	//! If your prepared SQL query has any ? placeholders, this is how you bind values to them.
	//! Placeholder numbers count from left to right and are 0-indexed.
	//!
	//! It is possible to use this function for bulk operations.
	//!
	//! \param param Placeholder position.
	//! \param elements The number of elements being bound.
	//! \throws database_error
	void bind_null(short param, std::size_t elements = 1);

	//! @}

	std::size_t get_result_set_count() const;

private:
	typedef std::function<bool(std::size_t)> null_predicate_type;

private:
	class statement_impl;
	std::shared_ptr<statement_impl> impl_;
};

} // dbc