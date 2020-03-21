#pragma once 

#include "common.hpp"

#include <memory>

namespace dbc
{

class catalog;
class statement;

//! \brief A resource for managing result sets from statement execution.
//!
//! \see statement::execute(), statement::execute_direct()
//! \note result objects may be copied, however all copies will refer to the same underlying ODBC result set.
class result
{
public:
	using ptr = std::shared_ptr<result>;

public:
	//! Empty result set.
	result();

	//! Free result set.
	~result() DBC_NOEXCEPT;

	//! \brief Returns the native ODBC statement handle.
	void* native_statement_handle() const;

	//! \brief The rowset size for this result set.
	long rowset_size() const DBC_NOEXCEPT;

	//! \brief Returns the number of rows affected by the request or -1 if the number of affected rows is not available.
	//! \throws database_error
	long affected_rows() const;

	//! \brief Returns the number of rows in the current rowset or 0 if the number of rows is not available.
	long rows() const DBC_NOEXCEPT;

	//! \brief Returns the number of columns in a result set.
	//! \throws database_error
	short columns() const;

	//! \brief Fetches the first row in the current result set.
	//! \return true if there are more results or false otherwise.
	//! \throws database_error
	bool first();

	//! \brief Fetches the last row in the current result set.
	//! \return true if there are more results or false otherwise.
	//! \throws database_error
	bool last();

	//! \brief Fetches the next row in the current result set.
	//! \return true if there are more results or false otherwise.
	//! \throws database_error
	bool next();

	//! \brief Fetches the prior row in the current result set.
	//! \return true if there are more results or false otherwise.
	//! \throws database_error
	bool prior();

	//! \brief Moves to and fetches the specified row in the current result set.
	//! \return true if there are results or false otherwise.
	//! \throws database_error
	bool move(long row);

	//! \brief Skips a number of rows and then fetches the resulting row in the current result set.
	//! \return true if there are results or false otherwise.
	//! \throws database_error
	bool skip(long rows);

	//! \brief Returns the row position in the current result set.
	unsigned long position() const;

	//! \brief Returns true if there are no more results in the current result set.
	bool end() const DBC_NOEXCEPT;

	//! \brief Gets data from the given column of the current rowset.
	//!
	//! Columns are numbered from left to right and 0-indexed.
	//! \param column position.
	//! \param result The column's value will be written to this parameter.
	//! \throws database_error, index_range_error, type_incompatible_error, null_access_error
	template<class T>
	void get_ref(short column, T& result) const;

	//! \brief Gets data from the given column of the current rowset.
	//! If the data is null, fallback is returned instead.
	//!
	//! Columns are numbered from left to right and 0-indexed.
	//! \param column position.
	//! \param fallback if value is null, return fallback instead.
	//! \param result The column's value will be written to this parameter.
	//! \throws database_error, index_range_error, type_incompatible_error
	template<class T>
	void get_ref(short column, const T& fallback, T& result) const;

	//! \brief Gets data from the given column by name of the current rowset.
	//!
	//! \param column_name column's name.
	//! \param result The column's value will be written to this parameter.
	//! \throws database_error, index_range_error, type_incompatible_error, null_access_error
	template<class T>
	void get_ref(const std::wstring& column_name, T& result) const;

	//! \brief Gets data from the given column by name of the current rowset.
	//! If the data is null, fallback is returned instead.
	//!
	//! \param column_name column's name.
	//! \param fallback if value is null, return fallback instead.
	//! \param result The column's value will be written to this parameter.
	//! \throws database_error, index_range_error, type_incompatible_error
	template<class T>
	void get_ref(const std::wstring& column_name, const T& fallback, T& result) const;

	//! \brief Gets data from the given column of the current rowset.
	//!
	//! Columns are numbered from left to right and 0-indexed.
	//! \param column position.
	//! \throws database_error, index_range_error, type_incompatible_error, null_access_error
	template<class T>
	T get(short column) const;

	//! \brief Gets data from the given column of the current rowset.
	//! If the data is null, fallback is returned instead.
	//!
	//! Columns are numbered from left to right and 0-indexed.
	//! \param column position.
	//! \param fallback if value is null, return fallback instead.
	//! \throws database_error, index_range_error, type_incompatible_error
	template<class T>
	T get(short column, const T& fallback) const;

	//! \brief Gets data from the given column by name of the current rowset.
	//!
	//! \param column_name column's name.
	//! \throws database_error, index_range_error, type_incompatible_error, null_access_error
	template<class T>
	T get(const std::wstring& column_name) const;

	//! \brief Gets data from the given column by name of the current rowset.
	//! If the data is null, fallback is returned instead.
	//!
	//! \param column_name column's name.
	//! \param fallback if value is null, return fallback instead.
	//! \throws database_error, index_range_error, type_incompatible_error
	template<class T>
	T get(const std::wstring& column_name, const T& fallback) const;

	//! \brief Returns true if and only if the given column of the current rowset is null.
	//!
	//! There is a bug/limitation in ODBC drivers for SQL Server (and possibly others)
	//! which causes SQLBindCol() to never write SQL_NOT_NULL to the length/indicator
	//! buffer unless you also bind the data column. Nanodbc's is_null() will return
	//! correct values for (n)varchar(max) columns when you ensure that SQLGetData()
	//! has been called for that column (i.e. after get() or get_ref() is called).
	//!
	//! Columns are numbered from left to right and 0-indexed.
	//! \see get(), get_ref()
	//! \param column position.
	//! \throws database_error, index_range_error
	bool is_null(short column) const;

	//! \brief Returns true if and only if the given column by name of the current rowset is null.
	//!
	//! See is_null(short column) for details on a bug/limitation of some ODBC drivers.
	//! \see is_null()
	//! \param column_name column's name.
	//! \throws database_error, index_range_error
	bool is_null(const std::wstring& column_name) const;

	//! \brief Returns the name of the specified column.
	//!
	//! Columns are numbered from left to right and 0-indexed.
	//! \param column position.
	//! \throws index_range_error
	std::wstring column_name(short column) const;

	//! \brief Returns the size of the specified column.
	//!
	//! Columns are numbered from left to right and 0-indexed.
	//! \param column position.
	//! \throws index_range_error
	long column_size(short column) const;

	//! \brief Returns the column number of the specified column name.
	//!
	//! Columns are numbered from left to right and 0-indexed.
	//! \param column_name column's name.
	//! \throws index_range_error
	short column(const std::wstring& column_name) const;

	//! Returns a identifying integer value representing the SQL type of this column.
	int column_datatype(short column) const;

	//! Returns a identifying integer value representing the SQL type of this column by name.
	int column_datatype(const std::wstring& column_name) const;

	//! Returns a identifying integer value representing the C type of this column.
	int column_c_datatype(short column) const;

	//! Returns a identifying integer value representing the C type of this column by name.
	int column_c_datatype(const std::wstring& column_name) const;

	//! If and only if result object is valid, returns true.
	explicit operator bool() const;

	//! Debug
	std::size_t get_column_reuse_count() const;

	// no move around
	result(const result& rhs) = delete;
	result(result&& rhs) = delete;
	result& operator=(result rhs) = delete;

public: // internal section
	void prepare_fetch(statement* statement, long rowset_size, bool changed, long rs_index);

private:
	class result_impl;
	std::shared_ptr<result_impl> impl_;
};

} // dbc