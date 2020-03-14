#pragma once

#include "idl_type_container.h"

#include <algorithm>
#include <cstdlib>
#include <sstream>

class idl_type_vec : public idl_type_container 
{
public:
	idl_type_vec()
	{
		type_ = type::vec;
	}

	~idl_type_vec()
	{
		delete value_type_;
	}

	// parser에 필요한 내용을 먼저 추가한다. 

	// 그 다음 생성에 필요한 내용을 추가한다.

	void set_value_type(idl_type* vtype)
	{
		WISE_ASSERT(vtype);

		value_type_ = vtype;

		std::ostringstream oss; 

		oss << "vec<";
		oss << value_type_->get_name();
		oss << ">";

		set_name(oss.str());
	}

	const idl_type* get_value_type() const
	{
		return value_type_;
	}

	void set_count(int count)
	{
		count_ = std::max(1, count);
	}

	int get_count() const 
	{
		return count_;
	}

private: 
	idl_type* value_type_ = nullptr;
	int count_ = 1;
};
