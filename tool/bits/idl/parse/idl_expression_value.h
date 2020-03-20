#pragma once

#include "idl_type_full.h"
#include "idl_type_simple.h"
#include <wise.kernel/core/macros.hpp>

/**
*
*/
class idl_expression_value
{
public: 
	enum class Type
	{
		None,
		Full, 
		Simple, 
		Const
	};
	
public:
	idl_expression_value()
	{
	}

	~idl_expression_value()
	{
		delete full_type_;
	}

	void set_scoped_id(idl_type_full* f, const std::string& id)
	{
		WISE_EXPECT(f);
		WISE_EXPECT(!id.empty());
		WISE_EXPECT(type_ == Type::None);
		WISE_EXPECT(full_type_ == nullptr);

		full_type_ = f; 
		id_ = id; 
		type_ = Type::Full;
	}

	void set_full_id(idl_type_full* f)
	{
		WISE_EXPECT(f);
		WISE_EXPECT(type_ == Type::None);
		WISE_EXPECT(full_type_ == nullptr);

		full_type_ = f; 
		type_ = Type::Full;
	}

	void set_simple_id(const std::string& id)
	{
		WISE_EXPECT(!id.empty());
		WISE_EXPECT(type_ == Type::None);
		WISE_EXPECT(id_.empty());

		id_ = id;
		type_ = Type::Simple;
	}

	void set_const_value(int v)
	{
		WISE_EXPECT(type_ == Type::None);
		ivalue_ = v; 
		type_ = Type::Const;
	}

	Type get_type() const
	{
		return type_;
	}

	const idl_type_full* get_full_type() const
	{
		return full_type_;
	}

	const std::string& get_id() const
	{
		return id_;
	}

	int get_value() const
	{
		return ivalue_;
	}

private: 
	Type type_ = Type::None;
	idl_type_full* full_type_ = nullptr; 
	std::string id_;
	int ivalue_ = 0;
};
