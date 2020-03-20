#include "stdafx.h"
#include "result.hpp"
#include "error.hpp"
#include "statement.hpp"

#include <wise/base/macros.hpp>
#include <spdlog/fmt/fmt.h>

#include <clocale>
#include <map>

namespace dbc
{

// Encapsulates resources needed for column binding.
class bound_column
{
public:
	bound_column(const bound_column& rhs) = delete;
	bound_column& operator=(bound_column rhs) = delete;

	bound_column()
		: name_()
		, column_(0)
		, sqltype_(0)
		, sqlsize_(0)
		, scale_(0)
		, ctype_(0)
		, clen_(0)
		, blob_(false)
		, cbdata_(0)
		, pdata_(0)
	{
	}

	~bound_column()
	{
		delete[] cbdata_;
		delete[] pdata_;
	}

public:
	std::wstring name_;
	short column_;
	SQLSMALLINT sqltype_;
	SQLULEN sqlsize_;
	SQLSMALLINT scale_;
	SQLSMALLINT ctype_;
	SQLULEN clen_;
	bool blob_;
	dbc::null_type* cbdata_ = nullptr;
	char* pdata_ = nullptr;
};


class result::result_impl
{
public:
	result_impl(const result_impl&) = delete;
	result_impl& operator=(const result_impl&) = delete;

	result_impl()
		: stmt_()
		, rowset_size_(0)
		, row_count_(0)
		, bound_columns_(0)
		, bound_columns_size_(0)
		, rowset_position_(0)
		, bound_columns_by_name_()
		, at_end_(false)
	{
	}

	void prepare_fetch(statement* stmt, long rowset_size, bool changed, long rs_index)
	{
		stmt_ = stmt;
		rowset_size_ = rowset_size;
		rs_changed_ = changed;
		rs_index_ = rs_index;

		DBC_ASSERT(stmt_);
		DBC_ASSERT(rowset_size_ >= 1);
		DBC_ASSERT(rs_index_ >= 0);

		RETCODE rc;

		// 한꺼번에 가져올 row의 개수를 지정한다. SQLFetchScroll을 사용할 때 지정한다. 
		// rowset_size_ 만큼씩 가져온다. 실제 가져온 개수는 row_count_에 지정된다. 
		DBC_CALL_RC(
			SQLSetStmtAttr
			, rc
			, stmt_->native_statement_handle()
			, SQL_ATTR_ROW_ARRAY_SIZE
			, (SQLPOINTER)(std::intptr_t)rowset_size_
			, 0);

		if (!success(rc))
		{
			DBC_THROW_DATABASE_ERROR(stmt_->native_statement_handle(), SQL_HANDLE_STMT);
		}

		// 가져올 row의 개수. 여기 호출 되었을 때 첫번째 자동 Fetch가 이루어지고 값이 지정됨
		// SQLMoreResults (next_result) 함수 호출에서도 지정됨
		DBC_CALL_RC(
			SQLSetStmtAttr
			, rc
			, stmt_->native_statement_handle()
			, SQL_ATTR_ROWS_FETCHED_PTR
			, &row_count_
			, 0);

		if (!success(rc))
		{
			DBC_THROW_DATABASE_ERROR(stmt_->native_statement_handle(), SQL_HANDLE_STMT);
		}

		auto_bind();
	}

	~result_impl() DBC_NOEXCEPT
	{
		cleanup_bound_columns();
	}

	void* native_statement_handle() const
	{
		return stmt_->native_statement_handle();
	}

	long rowset_size() const
	{
		return rowset_size_;
	}

	long affected_rows() const
	{
		return stmt_->affected_rows();
	}

	long rows() const DBC_NOEXCEPT
	{
		DBC_ASSERT(row_count_ <= static_cast<SQLULEN>(std::numeric_limits<long>::max()));

		return static_cast<long>(row_count_);
	}

	short columns() const
	{
		return stmt_->columns();
	}

	bool first()
	{
		WISE_RETURN_IF(!is_ready_to_fetch(), false);

		rowset_position_ = 0;

		return fetch(0, SQL_FETCH_FIRST);
	}

	bool last()
	{
		WISE_RETURN_IF(!is_ready_to_fetch(), false);

		rowset_position_ = 0;
		return fetch(0, SQL_FETCH_LAST);
	}

	bool next()
	{
		WISE_RETURN_IF(!is_ready_to_fetch(), false);

		if (rows() && ++rowset_position_ < rowset_size_)
		{
			return rowset_position_ < rows();
		}

		rowset_position_ = 0;

		return fetch(0, SQL_FETCH_NEXT);
	}

	bool prior()
	{
		WISE_RETURN_IF(!is_ready_to_fetch(), false);

		if (rows() && --rowset_position_ >= 0)
		{
			return true;
		}

		rowset_position_ = 0;

		return fetch(0, SQL_FETCH_PRIOR);
	}

	bool move(long row)
	{
		WISE_RETURN_IF(!is_ready_to_fetch(), false);

		rowset_position_ = 0;

		return fetch(row, SQL_FETCH_ABSOLUTE);
	}

	bool skip(long rows)
	{
		WISE_RETURN_IF(!is_ready_to_fetch(), false);

		rowset_position_ += rows;
		if (this->rows() && rowset_position_ < rowset_size_)
			return rowset_position_ < this->rows();
		rowset_position_ = 0;
		return fetch(rows, SQL_FETCH_RELATIVE);
	}

	unsigned long position() const
	{
		SQLULEN pos = 0; // necessary to initialize to 0
		RETCODE rc;
		DBC_CALL_RC(
			SQLGetStmtAttr
			, rc
			, stmt_->native_statement_handle()
			, SQL_ATTR_ROW_NUMBER
			, &pos
			, SQL_IS_UINTEGER
			, 0);

		if (!success(rc))
		{
			DBC_THROW_DATABASE_ERROR(stmt_->native_statement_handle(), SQL_HANDLE_STMT);
		}

		// MSDN (https://msdn.microsoft.com/en-us/library/ms712631.aspx):
		// If the number of the current row cannot be determined or
		// there is no current row, the driver returns 0.
		// Otherwise, valid row number is returned, starting at 1.
		//
		// NOTE: We try to address incorrect implementation in some drivers (e.g. SQLite ODBC)
		// which instead of 0 return SQL_ROW_NUMBER_UNKNOWN(-2) .
		if (pos == 0 || pos == static_cast<SQLULEN>(SQL_ROW_NUMBER_UNKNOWN))
		{
			return 0;
		}

		DBC_ASSERT(pos <= static_cast<SQLULEN>(std::numeric_limits<unsigned long>::max()));
		return static_cast<unsigned long>(pos) + rowset_position_;
	}

	bool end() const DBC_NOEXCEPT
	{
		if (at_end_)
		{
			return true;
		}

		SQLULEN pos = 0; // necessary to initialize to 0
		RETCODE rc;
		DBC_CALL_RC(
			SQLGetStmtAttr
			, rc
			, stmt_->native_statement_handle()
			, SQL_ATTR_ROW_NUMBER
			, &pos
			, SQL_IS_UINTEGER
			, 0);

		return (!success(rc) || rows() < 0 || pos - 1 > static_cast<unsigned long>(rows()));
	}

	bool is_null(short column) const
	{
		if (column >= bound_columns_size_)
		{
			throw index_range_error(
				fmt::format("index range error in is_null(). column: {}, bound_columns_size_ {}", 
					column, 
					bound_columns_size_).c_str()
			);
		}

		bound_column& col = bound_columns_[column];

		if (rowset_position_ >= rows())
		{
			throw index_range_error(
				fmt::format("index range error. rowset_position_ : {}, rows: {}", 
					rowset_position_, 
					rows()
				).c_str()
			);
		}

		return col.cbdata_[rowset_position_] == SQL_NULL_DATA;
	}

	bool is_null(const std::wstring& column_name) const
	{
		const short column = this->column(column_name);

		return is_null(column);
	}

	std::wstring column_name(short column) const
	{
		if (column >= bound_columns_size_)
		{
			throw index_range_error(
				fmt::format("index range error in column_name(). column: {}, bound_columns_size_ {}",
					column,
					bound_columns_size_).c_str()
			);
		}

		return bound_columns_[column].name_;
	}

	long column_size(short column) const
	{
		if (column >= bound_columns_size_)
		{
			throw index_range_error(
				fmt::format("index range error in column_size(). column: {}, bound_columns_size_ {}",
					column,
					bound_columns_size_).c_str()
			);
		}

		bound_column& col = bound_columns_[column];

		DBC_ASSERT(col.sqlsize_ <= static_cast<SQLULEN>(std::numeric_limits<long>::max()));

		return static_cast<long>(col.sqlsize_);
	}

	short column(const std::wstring& column_name) const
	{
		typedef std::map<std::wstring, bound_column*>::const_iterator iter;

		iter i = bound_columns_by_name_.find(column_name);

		if (i == bound_columns_by_name_.end())
		{
			std::string cn; 
			convert(column_name, cn);

			throw index_range_error(
				fmt::format("index range error in column(). column_name: {}", 
					cn
				).c_str()
			);
		}

		return i->second->column_;
	}

	int column_datatype(short column) const
	{
		if (column >= bound_columns_size_)
		{
			throw index_range_error(
				fmt::format("index range error in column_datatype(). column: {}, bound_columns_size_ {}",
					column,
					bound_columns_size_).c_str()
			);
		}

		bound_column& col = bound_columns_[column];

		return col.sqltype_;
	}

	int column_datatype(const std::wstring& column_name) const
	{
		const short column = this->column(column_name);

		bound_column& col = bound_columns_[column];

		return col.sqltype_;
	}

	int column_c_datatype(short column) const
	{
		if (column >= bound_columns_size_)
		{
			throw index_range_error(
				fmt::format("index range error in column_c_datatype(). column: {}, bound_columns_size_ {}",
					column,
					bound_columns_size_).c_str()
			);
		}

		bound_column& col = bound_columns_[column];

		return col.ctype_;
	}

	int column_c_datatype(const std::wstring& column_name) const
	{
		const short column = this->column(column_name);

		bound_column& col = bound_columns_[column];

		return col.ctype_;
	}

	template<class T>
	void get_ref(short column, T& result) const
	{
		if (column >= bound_columns_size_)
		{
			throw index_range_error(
				fmt::format("index range error in get_ref(column, result). column: {}, bound_columns_size_ {}",
					column,
					bound_columns_size_).c_str()
			);
		}

		if (is_null(column))
		{
			throw null_access_error(
				fmt::format("null access error in get_ref(column, result). column: {}", 
					column
				).c_str()
			);
		}

		get_ref_impl<T>(column, result);
	}

	template<class T>
	void get_ref(short column, const T& fallback, T& result) const
	{
		if (column >= bound_columns_size_)
		{
			throw index_range_error(
				fmt::format("index range error in get_ref(column, fallback, result). column: {}, bound_columns_size_ {}",
					column,
					bound_columns_size_).c_str()
			);
		}

		if (is_null(column))
		{
			result = fallback;
			return;
		}

		get_ref_impl<T>(column, result);
	}

	template<class T>
	void get_ref(const std::wstring& column_name, T& result) const
	{
		const short column = this->column(column_name);

		if (is_null(column))
		{
			throw null_access_error(
				fmt::format("null access error in get_ref(column_name, result). column: {}",
					column
				).c_str()
			);
		}

		get_ref_impl<T>(column, result);
	}

	template<class T>
	void get_ref(const std::wstring& column_name, const T& fallback, T& result) const
	{
		const short column = this->column(column_name);

		if (is_null(column))
		{
			result = fallback;
			return;
		}

		get_ref_impl<T>(column, result);
	}

	template<class T>
	T get(short column) const
	{
		T result;
		get_ref(column, result);
		return result;
	}

	template<class T>
	T get(short column, const T& fallback) const
	{
		T result;
		get_ref(column, fallback, result);
		return result;
	}

	template<class T>
	T get(const std::wstring& column_name) const
	{
		T result;
		get_ref(column_name, result);
		return result;
	}

	template<class T>
	T get(const std::wstring& column_name, const T& fallback) const
	{
		T result;
		get_ref(column_name, fallback, result);
		return result;
	}

	std::size_t get_column_reuse_count() const
	{
		return column_reuse_count_;
	}

private:
	template<class T>
	void get_ref_impl(short column, T& result) const;

	template<class STR> 
	void get_ref_impl_string(short column, STR& result) const;

	void before_move() DBC_NOEXCEPT
	{
		for (short i = 0; i < bound_columns_size_; ++i)
		{
			bound_column& col = bound_columns_[i];

			for (long j = 0; j < rowset_size_; ++j)
			{
				col.cbdata_[j] = 0;
			}

			// blob일 경우 pdata_가 있으면 해제한다. 
			// blob_일 경우 pdata_가 유효한 경우가 없으므로 코드 없앰
		}
	}

	void cleanup_bound_columns() DBC_NOEXCEPT
	{
		for (short i = 0; i < bound_columns_size_; ++i)
		{
			bound_column& col = bound_columns_[i];

			delete col.cbdata_;
			delete col.pdata_;

			col.cbdata_ = 0; 
			col.pdata_ = 0;
		}

		delete[] bound_columns_;

		bound_columns_ = NULL;
		bound_columns_size_ = 0;
		bound_columns_by_name_.clear();
	}

	bool fetch(long rows, SQLUSMALLINT orientation)
	{
		before_move();
		RETCODE rc;

		DBC_CALL_RC(
			SQLFetchScroll
			, rc
			, stmt_->native_statement_handle()
			, orientation
			, rows);

		if (rc == SQL_NO_DATA)
		{
			at_end_ = true;
			return false;
		}

		if (!success(rc))
		{
			DBC_THROW_DATABASE_ERROR(stmt_->native_statement_handle(), SQL_HANDLE_STMT);
		}

		return true;
	}

	void auto_bind()
	{
		is_bound_ = false;

		const short n_columns = columns();

		if (n_columns < 1)
		{
			return;
		}

		if (rs_changed_)
		{
			column_reuse_count_ = 0;

			cleanup_bound_columns();

			DBC_ASSERT(!bound_columns_);
			DBC_ASSERT(!bound_columns_size_);
			bound_columns_ = new bound_column[n_columns];
			bound_columns_size_ = n_columns;
		}
		else
		{
			column_reuse_count_++;
		}

		RETCODE rc;
		DBC_SQLCHAR column_name[1024];
		SQLSMALLINT sqltype, scale, nullable, len;
		SQLULEN sqlsize;

		for (SQLSMALLINT i = 0; i < n_columns; ++i)
		{
			DBC_CALL_RC(
				DBC_FUNC(SQLDescribeCol)
				, rc
				, stmt_->native_statement_handle()
				, i + 1
				, (DBC_SQLCHAR*)column_name
				, sizeof(column_name) / sizeof(DBC_SQLCHAR)
				, &len
				, &sqltype
				, &sqlsize
				, &scale
				, &nullable);

			if (!success(rc))
			{
				DBC_THROW_DATABASE_ERROR(stmt_->native_statement_handle(), SQL_HANDLE_STMT);
			}

			// Adjust the sqlsize parameter in case of "unlimited" data (varchar(max), nvarchar(max)).
			bool is_blob = false;

			if (sqlsize == 0)
			{
				// sqlsize 가 0이면 blob (binary large object)이다.

				switch (sqltype)
				{
				case SQL_VARCHAR:
				case SQL_WVARCHAR:
				{
					is_blob = true;
				}
				}
			}

			bound_column& col = bound_columns_[i];
			col.name_ = reinterpret_cast<std::wstring::value_type*>(column_name);
			col.column_ = i;
			col.sqltype_ = sqltype;
			col.sqlsize_ = sqlsize;
			col.scale_ = scale;
			bound_columns_by_name_[col.name_] = &col;

			using namespace std; // if int64_t is in std namespace (in c++11)
			switch (col.sqltype_)
			{
			case SQL_BIT:
			case SQL_TINYINT:
			case SQL_SMALLINT:
			case SQL_INTEGER:
				col.ctype_ = SQL_C_LONG;
				col.clen_ = sizeof(int32_t);
				break;
			case SQL_BIGINT:
				col.ctype_ = SQL_C_SBIGINT;
				col.clen_ = sizeof(int64_t);
				break;
			case SQL_DOUBLE:
			case SQL_FLOAT:
			case SQL_DECIMAL:
			case SQL_REAL:
			case SQL_NUMERIC:
				col.ctype_ = SQL_C_DOUBLE;
				col.clen_ = sizeof(double);
				break;
			case SQL_DATE:
			case SQL_TYPE_DATE:
				col.ctype_ = SQL_C_DATE;
				col.clen_ = sizeof(date);
				break;
			case SQL_TIMESTAMP:
			case SQL_TYPE_TIMESTAMP:
				col.ctype_ = SQL_C_TIMESTAMP;
				col.clen_ = sizeof(timestamp);
				break;
			case SQL_CHAR:
			case SQL_VARCHAR:
				col.ctype_ = SQL_C_CHAR;
				col.clen_ = (col.sqlsize_ + 1) * sizeof(SQLCHAR);
				if (is_blob)
				{
					col.clen_ = 0;
					col.blob_ = true;
				}
				break;
			case SQL_WCHAR:
			case SQL_WVARCHAR:
				col.ctype_ = SQL_C_WCHAR;
				col.clen_ = (col.sqlsize_ + 1) * sizeof(SQLWCHAR);
				if (is_blob)
				{
					col.clen_ = 0;
					col.blob_ = true;
				}
				break;
			case SQL_LONGVARCHAR:
				col.ctype_ = SQL_C_CHAR;
				col.blob_ = true;
				col.clen_ = 0;
				break;
			case SQL_BINARY:
			case SQL_VARBINARY:
			case SQL_LONGVARBINARY:
				col.ctype_ = SQL_C_BINARY;
				col.blob_ = true;
				col.clen_ = 0;
				break;
			default:
				col.ctype_ = sql_ctype<std::wstring>::value;
				col.clen_ = 128;
				break;
			}
		}

		for (SQLSMALLINT i = 0; i < n_columns; ++i)
		{
			bound_column& col = bound_columns_[i];

			if (rs_changed_)
			{
				col.cbdata_ = new null_type[rowset_size_];
			}

			if (col.blob_)
			{
				//
				// blob에 대해 메모리를 할당하여 제공하지 않는다. 
				// SQLGetData()로 버퍼를 제공해서 가져온다. 
				// 현재 1024로 고정된 버퍼로 문자열로 get 하는 함수만 제공된다. 
				// 

				DBC_CALL_RC(
					SQLBindCol
					, rc
					, stmt_->native_statement_handle()
					, i + 1
					, col.ctype_
					, 0
					, 0
					, col.cbdata_);
				if (!success(rc))
					DBC_THROW_DATABASE_ERROR(stmt_->native_statement_handle(), SQL_HANDLE_STMT);
			}
			else
			{
				if (rs_changed_)
				{
					col.pdata_ = new char[rowset_size_ * col.clen_];
				}

				DBC_CALL_RC(
					SQLBindCol
					, rc
					, stmt_->native_statement_handle()
					, i + 1         // ColumnNumber
					, col.ctype_    // TargetType
					, col.pdata_    // TargetValuePtr
					, col.clen_     // BufferLength
					, col.cbdata_); // StrLen_or_Ind
				if (!success(rc))
					DBC_THROW_DATABASE_ERROR(stmt_->native_statement_handle(), SQL_HANDLE_STMT);
			}
		}

		is_bound_ = true;
	}

	bool is_ready_to_fetch() const
	{
		return	bound_columns_ != nullptr && rowset_size_ > 0 &&
			rs_index_ >= 0 && is_bound_;
	}

private:
	statement* stmt_ = nullptr;
	long rowset_size_;
	SQLULEN row_count_;
	bound_column* bound_columns_;
	short bound_columns_size_;
	long rowset_position_;
	std::map<std::wstring, bound_column*> bound_columns_by_name_;
	bool at_end_;
	bool rs_changed_;
	long rs_index_ = -1;
	bool is_bound_ = false; 
	std::size_t column_reuse_count_ = 0;
};

result::result()
	: impl_(new result_impl())
{
}

result::~result() DBC_NOEXCEPT
{
	// 메모리 도구 friendly
	impl_.reset();
}

void result::prepare_fetch(statement* stmt, long rowset_size, bool changed, long rs_index)
{
	impl_->prepare_fetch(stmt, rowset_size, changed, rs_index);
}

void* result::native_statement_handle() const
{
	return impl_->native_statement_handle();
}

long result::rowset_size() const DBC_NOEXCEPT
{
	return impl_->rowset_size();
}

long result::affected_rows() const
{
	return impl_->affected_rows();
}

long result::rows() const DBC_NOEXCEPT
{
	return impl_->rows();
}

short result::columns() const
{
	return impl_->columns();
}

bool result::first()
{
	return impl_->first();
}

bool result::last()
{
	return impl_->last();
}

bool result::next()
{
	return impl_->next();
}

bool result::prior()
{
	return impl_->prior();
}

bool result::move(long row)
{
	return impl_->move(row);
}

bool result::skip(long rows)
{
	return impl_->skip(rows);
}

unsigned long result::position() const
{
	return impl_->position();
}

bool result::end() const DBC_NOEXCEPT
{
	return impl_->end();
}

bool result::is_null(short column) const
{
	return impl_->is_null(column);
}

bool result::is_null(const std::wstring& column_name) const
{
	return impl_->is_null(column_name);
}

std::wstring result::column_name(short column) const
{
	return impl_->column_name(column);
}

long result::column_size(short column) const
{
	return impl_->column_size(column);
}

short result::column(const std::wstring& column_name) const
{
	return impl_->column(column_name);
}

int result::column_datatype(short column) const
{
	return impl_->column_datatype(column);
}

int result::column_datatype(const std::wstring& column_name) const
{
	return impl_->column_datatype(column_name);
}

int result::column_c_datatype(short column) const
{
	return impl_->column_c_datatype(column);
}

int result::column_c_datatype(const std::wstring& column_name) const
{
	return impl_->column_c_datatype(column_name);
}

template<class T>
void result::get_ref(short column, T& result) const
{
	return impl_->get_ref<T>(column, result);
}

template<class T>
void result::get_ref(short column, const T& fallback, T& result) const
{
	return impl_->get_ref<T>(column, fallback, result);
}

template<class T>
void result::get_ref(const std::wstring& column_name, T& result) const
{
	return impl_->get_ref<T>(column_name, result);
}

template<class T>
void result::get_ref(const std::wstring& column_name, const T& fallback, T& result) const
{
	return impl_->get_ref<T>(column_name, fallback, result);
}

template<class T>
T result::get(short column) const
{
	return impl_->get<T>(column);
}

template<class T>
T result::get(short column, const T& fallback) const
{
	return impl_->get<T>(column, fallback);
}

template<class T>
T result::get(const std::wstring& column_name) const
{
	return impl_->get<T>(column_name);
}

template<class T>
T result::get(const std::wstring& column_name, const T& fallback) const
{
	return impl_->get<T>(column_name, fallback);
}

result::operator bool() const
{
	return static_cast<bool>(impl_);
}

std::size_t result::get_column_reuse_count() const
{
	return impl_->get_column_reuse_count();
}

// The following are the only supported instantiations of result::get_ref().
template void result::get_ref(short, bool&) const;
template void result::get_ref(short, int8_t&) const;
template void result::get_ref(short, uint8_t&) const;
template void result::get_ref(short, std::wstring::value_type&) const;
template void result::get_ref(short, short&) const;
template void result::get_ref(short, unsigned short&) const;
template void result::get_ref(short, int32_t&) const;
template void result::get_ref(short, uint32_t&) const;
template void result::get_ref(short, int64_t&) const;
template void result::get_ref(short, uint64_t&) const;
template void result::get_ref(short, float&) const;
template void result::get_ref(short, double&) const;
template void result::get_ref(short, std::wstring&) const;
template void result::get_ref(short, std::string&) const;
template void result::get_ref(short, date&) const;
template void result::get_ref(short, timestamp&) const;

template void result::get_ref(const std::wstring&, std::wstring::value_type&) const;
template void result::get_ref(const std::wstring&, short&) const;
template void result::get_ref(const std::wstring&, unsigned short&) const;
template void result::get_ref(const std::wstring&, int32_t&) const;
template void result::get_ref(const std::wstring&, uint32_t&) const;
template void result::get_ref(const std::wstring&, int64_t&) const;
template void result::get_ref(const std::wstring&, uint64_t&) const;
template void result::get_ref(const std::wstring&, float&) const;
template void result::get_ref(const std::wstring&, double&) const;
template void result::get_ref(const std::wstring&, std::wstring&) const;
template void result::get_ref(const std::wstring&, date&) const;
template void result::get_ref(const std::wstring&, timestamp&) const;

// The following are the only supported instantiations of result::get_ref() with fallback.
template void result::get_ref(short, const std::wstring::value_type&, std::wstring::value_type&) const;
template void result::get_ref(short, const short&, short&) const;
template void result::get_ref(short, const unsigned short&, unsigned short&) const;
template void result::get_ref(short, const int32_t&, int32_t&) const;
template void result::get_ref(short, const uint32_t&, uint32_t&) const;
template void result::get_ref(short, const int64_t&, int64_t&) const;
template void result::get_ref(short, const uint64_t&, uint64_t&) const;
template void result::get_ref(short, const float&, float&) const;
template void result::get_ref(short, const double&, double&) const;
template void result::get_ref(short, const std::wstring&, std::wstring&) const;
template void result::get_ref(short, const std::string&, std::string&) const;
template void result::get_ref(short, const date&, date&) const;
template void result::get_ref(short, const timestamp&, timestamp&) const;

template void result::get_ref(const std::wstring&, const std::wstring::value_type&, std::wstring::value_type&) const;
template void result::get_ref(const std::wstring&, const short&, short&) const;
template void result::get_ref(const std::wstring&, const unsigned short&, unsigned short&) const;
template void result::get_ref(const std::wstring&, const int32_t&, int32_t&) const;
template void result::get_ref(const std::wstring&, const uint32_t&, uint32_t&) const;
template void result::get_ref(const std::wstring&, const int64_t&, int64_t&) const;
template void result::get_ref(const std::wstring&, const uint64_t&, uint64_t&) const;
template void result::get_ref(const std::wstring&, const float&, float&) const;
template void result::get_ref(const std::wstring&, const double&, double&) const;
template void result::get_ref(const std::wstring&, const std::wstring&, std::wstring&) const;
template void result::get_ref(const std::wstring&, const std::string&, std::string&) const;
template void result::get_ref(const std::wstring&, const date&, date&) const;
template void result::get_ref(const std::wstring&, const timestamp&, timestamp&) const;

// The following are the only supported instantiations of result::get().
template std::wstring::value_type result::get(short) const;
template bool result::get(short) const;
template int8_t result::get(short) const;
template uint8_t result::get(short) const;
template short result::get(short) const;
template unsigned short result::get(short) const;
template int32_t result::get(short) const;
template uint32_t result::get(short) const;
template long result::get(short) const;
template int64_t result::get(short) const;
template uint64_t result::get(short) const;
template float result::get(short) const;
template double result::get(short) const;
template std::wstring result::get(short) const;
template std::string result::get(short) const;
template date result::get(short) const;
template timestamp result::get(short) const;

template std::wstring::value_type result::get(const std::wstring&) const;
template short result::get(const std::wstring&) const;
template unsigned short result::get(const std::wstring&) const;
template int32_t result::get(const std::wstring&) const;
template uint32_t result::get(const std::wstring&) const;
template long result::get(const std::wstring&) const;
template int64_t result::get(const std::wstring&) const;
template uint64_t result::get(const std::wstring&) const;
template float result::get(const std::wstring&) const;
template double result::get(const std::wstring&) const;
template std::wstring result::get(const std::wstring&) const;
template std::string result::get(const std::wstring&) const;
template date result::get(const std::wstring&) const;
template timestamp result::get(const std::wstring&) const;

// The following are the only supported instantiations of result::get() with fallback.
template std::wstring::value_type result::get(short, const std::wstring::value_type&) const;
template short result::get(short, const short&) const;
template unsigned short result::get(short, const unsigned short&) const;
template int32_t result::get(short, const int32_t&) const;
template uint32_t result::get(short, const uint32_t&) const;
template long result::get(short, const long&) const;
template int64_t result::get(short, const int64_t&) const;
template uint64_t result::get(short, const uint64_t&) const;
template float result::get(short, const float&) const;
template double result::get(short, const double&) const;
template std::wstring result::get(short, const std::wstring&) const;
template std::string result::get(short, const std::string&) const;
template date result::get(short, const date&) const;
template timestamp result::get(short, const timestamp&) const;

template std::wstring::value_type result::get(const std::wstring&, const std::wstring::value_type&) const;
template short result::get(const std::wstring&, const short&) const;
template unsigned short result::get(const std::wstring&, const unsigned short&) const;
template int32_t result::get(const std::wstring&, const int32_t&) const;
template uint32_t result::get(const std::wstring&, const uint32_t&) const;
template int64_t result::get(const std::wstring&, const int64_t&) const;
template uint64_t result::get(const std::wstring&, const uint64_t&) const;
template float result::get(const std::wstring&, const float&) const;
template double result::get(const std::wstring&, const double&) const;
template std::wstring result::get(const std::wstring&, const std::wstring&) const;
template std::string result::get(const std::wstring&, const std::string&) const;
template date result::get(const std::wstring&, const date&) const;
template timestamp result::get(const std::wstring&, const timestamp&) const;


template<>
inline void result::result_impl::get_ref_impl<bool>(short column, bool& result) const
{
	bound_column& col = bound_columns_[column];

	const char* s = col.pdata_ + rowset_position_ * col.clen_;

	switch (col.ctype_)
	{
	case SQL_C_BIT:
	case SQL_C_LONG:	// To support implicit conversion in SELECT.
		result = (bool)*(bool*)(s);
		return;
	}

	throw type_incompatible_error(
		fmt::format("type incompatabile error in get_ref_impl<bool>()").c_str()
	);
}


template<>
inline void result::result_impl::get_ref_impl<date>(short column, date& result) const
{
	bound_column& col = bound_columns_[column];
	switch (col.ctype_)
	{
	case SQL_C_DATE:
		result = *reinterpret_cast<date*>(col.pdata_ + rowset_position_ * col.clen_);
		return;
	case SQL_C_TIMESTAMP:
	{
		timestamp stamp = *reinterpret_cast<timestamp*>(col.pdata_ + rowset_position_ * col.clen_);
		date d; 
		d.year = stamp.year;
		d.month = stamp.month;
		d.day = stamp.day;
		result = d;
		return;
	}
	}

	throw type_incompatible_error(
		fmt::format("type incompatabile error in get_ref_impl<date>()").c_str()
	);
}

template<>
inline void result::result_impl::get_ref_impl<timestamp>(short column, timestamp& result) const
{
	bound_column& col = bound_columns_[column];
	switch (col.ctype_)
	{
	case SQL_C_DATE:
	{
		date d = *reinterpret_cast<date*>(col.pdata_ + rowset_position_ * col.clen_);

		timestamp stamp;
		stamp.year = d.year; 
		stamp.month = d.month;
		stamp.day = d.day;

		result = stamp;
		return;
	}
	case SQL_C_TIMESTAMP:
		result = *reinterpret_cast<timestamp*>(col.pdata_ + rowset_position_ * col.clen_);
		return;
	}

	throw type_incompatible_error(
		fmt::format("type incompatabile error in get_ref_impl<timestamp>()").c_str()
	);
}

template<>
inline void result::result_impl::get_ref_impl<std::wstring>(short column, std::wstring& result) const
{
	get_ref_impl_string<std::wstring>(column, result);
}

template<>
inline void result::result_impl::get_ref_impl<std::string>(short column, std::string& result) const
{
	get_ref_impl_string<std::string>(column, result);
}

template<class STR>
inline void result::result_impl::get_ref_impl_string(short column, STR& result) const
{
	bound_column& col = bound_columns_[column];
	const SQLULEN column_size = col.sqlsize_;

	switch (col.ctype_)
	{
	case SQL_C_CHAR:
	case SQL_C_BINARY:
	{
		if (col.blob_)
		{
			// Input is always std::string, while output may be std::string or std::wstring
			std::string out;
			SQLLEN ValueLenOrInd;
			SQLRETURN rc;
			void* handle = native_statement_handle();
			do
			{
				char buffer[1024] = { 0 };
				const std::size_t buffer_size = sizeof(buffer);

				DBC_CALL_RC(
					SQLGetData
					, rc
					, handle            // StatementHandle
					, column + 1        // Col_or_Param_Num
					, col.ctype_        // TargetType
					, buffer            // TargetValuePtr
					, buffer_size - 1   // BufferLength
					, &ValueLenOrInd);  // StrLen_or_IndPtr

				if (ValueLenOrInd > 0)
				{
					out.append(buffer);
				}
				else if (ValueLenOrInd == SQL_NULL_DATA)
				{
					*col.cbdata_ = (SQLINTEGER)SQL_NULL_DATA;
				}

				// Sequence of successful calls is:
				// SQL_NO_DATA or SQL_SUCCESS_WITH_INFO followed by SQL_SUCCESS.
			} while (rc == SQL_SUCCESS_WITH_INFO);

			if (rc == SQL_SUCCESS || rc == SQL_NO_DATA)
			{
				convert(out, result);
			}
		}
		else
		{
			const char* s = col.pdata_ + rowset_position_ * col.clen_;
			const std::string::size_type str_size = std::strlen(s);
			result.assign(s, s + str_size);
		}
		return;
	}

	case SQL_C_WCHAR:
	{
		if (col.blob_)
		{
			// Input is always std::wstring, output might be std::string or std::wstring.
			// Use a string builder to build the output string.
			std::wstring out;
			SQLLEN ValueLenOrInd;
			SQLRETURN rc;
			void* handle = native_statement_handle();
			do
			{
				wchar_t buffer[512] = { 0 };
				const std::size_t buffer_size = sizeof(buffer);
				DBC_CALL_RC(
					SQLGetData
					, rc
					, handle            // StatementHandle
					, column + 1        // Col_or_Param_Num
					, col.ctype_        // TargetType
					, buffer            // TargetValuePtr
					, buffer_size - 1   // BufferLength
					, &ValueLenOrInd);  // StrLen_or_IndPtr
				if (ValueLenOrInd > 0)
					out.append(buffer);
				else if (ValueLenOrInd == SQL_NULL_DATA)
					*col.cbdata_ = (SQLINTEGER)SQL_NULL_DATA;
				// Sequence of successful calls is:
				// SQL_NO_DATA or SQL_SUCCESS_WITH_INFO followed by SQL_SUCCESS.
			} while (rc == SQL_SUCCESS_WITH_INFO);
			if (rc == SQL_SUCCESS || rc == SQL_NO_DATA)
				convert(out, result);
		}
		else
		{
			// Type is unicode in the database, convert if necessary
			const SQLWCHAR* s = reinterpret_cast<SQLWCHAR*>(col.pdata_ + rowset_position_ * col.clen_);
			const std::wstring::size_type str_size = *col.cbdata_ / sizeof(SQLWCHAR);
			std::wstring temp(s, s + str_size);
			convert(temp, result);
		}
		return;
	}

	case SQL_C_GUID:
	{
		const char* s = col.pdata_ + rowset_position_ * col.clen_;
		result.assign(s, s + column_size);
		return;
	}

	case SQL_C_LONG:
	{
		std::string buffer;
		buffer.reserve(column_size + 1); // ensure terminating null
		buffer.resize(buffer.capacity());
		using std::fill;
		fill(buffer.begin(), buffer.end(), '\0');
		const wchar_t data = *reinterpret_cast<wchar_t*>(col.pdata_ + rowset_position_ * col.clen_);
		const int bytes = std::snprintf(const_cast<char*>(buffer.data()), column_size, "%d", static_cast<int>(data));

		if (bytes == -1)
		{
			throw type_incompatible_error(
				fmt::format("type incompatabile error in get_ref_impl_string(). type: SQL_C_LONG").c_str()
			);
		}
		else if ((SQLULEN)bytes < column_size)
		{
			buffer.resize(bytes);
		}

		buffer.resize(std::strlen(buffer.data())); // drop any trailing nulls
		result.reserve(buffer.size() * sizeof(std::wstring::value_type));
		convert(buffer, result);
		return;
	}

	case SQL_C_SBIGINT:
	{
		using namespace std; // in case intmax_t is in namespace std
		std::string buffer;
		buffer.reserve(column_size + 1); // ensure terminating null
		buffer.resize(buffer.capacity());
		using std::fill;
		fill(buffer.begin(), buffer.end(), '\0');
		const intmax_t data = (intmax_t)*reinterpret_cast<int64_t*>(col.pdata_ + rowset_position_ * col.clen_);
		const int bytes = std::snprintf(const_cast<char*>(buffer.data()), column_size, "%jd", data);

		if (bytes == -1)
		{
			throw type_incompatible_error(
				fmt::format("type incompatabile error in get_ref_impl_string(). type: SQL_C_SBIGINT").c_str()
			);
		}
		else if ((SQLULEN)bytes < column_size)
		{
			buffer.resize(bytes);
		}

		buffer.resize(std::strlen(buffer.data())); // drop any trailing nulls
		result.reserve(buffer.size() * sizeof(std::wstring::value_type));
		convert(buffer, result);
		return;
	}

	case SQL_C_FLOAT:
	{
		std::string buffer;
		buffer.reserve(column_size + 1); // ensure terminating null
		buffer.resize(buffer.capacity());
		using std::fill;
		fill(buffer.begin(), buffer.end(), '\0');
		const float data = *reinterpret_cast<float*>(col.pdata_ + rowset_position_ * col.clen_);
		const int bytes = std::snprintf(const_cast<char*>(buffer.data()), column_size, "%f", data);
		if (bytes == -1)
		{
			throw type_incompatible_error(
				fmt::format("type incompatabile error in get_ref_impl_string(). type: SQL_C_FLOAT").c_str()
			);
		}
		else if ((SQLULEN)bytes < column_size)
		{
			buffer.resize(bytes);
		}

		buffer.resize(std::strlen(buffer.data())); // drop any trailing nulls
		result.reserve(buffer.size() * sizeof(std::wstring::value_type));
		convert(buffer, result);
		return;
	}

	case SQL_C_DOUBLE:
	{
		std::string buffer;
		const SQLULEN width = column_size + 2; // account for decimal mark and sign
		buffer.reserve(width + 1); // ensure terminating null
		buffer.resize(buffer.capacity());
		using std::fill;
		fill(buffer.begin(), buffer.end(), '\0');
		const double data = *reinterpret_cast<double*>(col.pdata_ + rowset_position_ * col.clen_);
		const int bytes = std::snprintf(
			const_cast<char*>(buffer.data())
			, width
			, "%.*lf"                       // restrict the number of digits
			, col.scale_                    // number of digits after the decimal point
			, data);

		if (bytes == -1)
		{
			throw type_incompatible_error(
				fmt::format("type incompatabile error in get_ref_impl_string(). type: SQL_C_DOUBLE").c_str()
			);
		}
		else if ((SQLULEN)bytes < column_size)
		{
			buffer.resize(bytes);
		}

		buffer.resize(std::strlen(buffer.data())); // drop any trailing nulls
		result.reserve(buffer.size() * sizeof(std::wstring::value_type));
		convert(buffer, result);
		return;
	}

	case SQL_C_DATE:
	{
		const date d = *reinterpret_cast<date*>(col.pdata_ + rowset_position_ * col.clen_);
		std::tm st = { 0 };
		st.tm_year = d.year - 1900;
		st.tm_mon = d.month - 1;
		st.tm_mday = d.day;
		char* old_lc_time = std::setlocale(LC_TIME, NULL);
		std::setlocale(LC_TIME, "");
		char date_str[512];
		std::strftime(date_str, sizeof(date_str), "%Y-%m-%d", &st);
		std::setlocale(LC_TIME, old_lc_time);
		convert(date_str, result);
		return;
	}

	case SQL_C_TIMESTAMP:
	{
		const timestamp stamp = *reinterpret_cast<timestamp*>(col.pdata_ + rowset_position_ * col.clen_);
		std::tm st = { 0 };
		st.tm_year = stamp.year - 1900;
		st.tm_mon = stamp.month - 1;
		st.tm_mday = stamp.day;
		st.tm_hour = stamp.hour;
		st.tm_min = stamp.min;
		st.tm_sec = stamp.sec;
		char* old_lc_time = std::setlocale(LC_TIME, NULL);
		std::setlocale(LC_TIME, "");
		char date_str[512];
		std::strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M:%S %z", &st);
		std::setlocale(LC_TIME, old_lc_time);
		convert(date_str, result);
		return;
	}
	}

	throw type_incompatible_error(
		fmt::format("type incompatabile error in get_ref_impl_string()").c_str()
	);
}

template<class T>
void result::result_impl::get_ref_impl(short column, T& result) const
{
	bound_column& col = bound_columns_[column];
	using namespace std; // if int64_t is in std namespace (in c++11)
	const char* s = col.pdata_ + rowset_position_ * col.clen_;
	switch (col.ctype_)
	{
	case SQL_C_CHAR: result = (T)*(char*)(s); return;
	case SQL_C_SSHORT: result = (T)*(short*)(s); return;
	case SQL_C_USHORT: result = (T)*(unsigned short*)(s); return;
	case SQL_C_LONG: result = (T)*(int32_t*)(s); return;
	case SQL_C_SLONG: result = (T)*(int32_t*)(s); return;
	case SQL_C_ULONG: result = (T)*(uint32_t*)(s); return;
	case SQL_C_FLOAT: result = (T)*(float*)(s); return;
	case SQL_C_DOUBLE: result = (T)*(double*)(s); return;
	case SQL_C_SBIGINT: result = (T)*(int64_t*)(s); return;
	case SQL_C_UBIGINT: result = (T)*(uint64_t*)(s); return;
	}

	throw type_incompatible_error(
		fmt::format("type incompatabile error in get_ref_impl()").c_str()
	);
}

} // dbc