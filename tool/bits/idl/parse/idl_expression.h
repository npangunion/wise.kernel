#pragma once

#include "idl_expression_value.h"
#include <wise.kernel/core/macros.hpp>
#include <vector>

/**
 * 
 */
class idl_expression
{
public: 
	enum Op
	{
		None, 
		Plus, 
		Minus
	};

public:
	idl_expression() = default;

	~idl_expression()
	{
		delete value_;

		for (auto& p : exprs_)
		{
			delete p;
		}

		exprs_.clear();
	}

	void add_plus(idl_expression* next)
	{
		WISE_EXPECT(next);

		next->set_op_link(Plus);

		exprs_.push_back(next);
	}

	void add_minus(idl_expression* next)
	{
		WISE_EXPECT(next);

		next->set_op_link(Minus);

		exprs_.push_back(next);
	}

	void set_value(idl_expression_value* value)
	{
		WISE_EXPECT(value);
		WISE_EXPECT(value_ == nullptr);

		value_ = value;
	}

	Op get_operator() const
	{
		return op_;
	}

	const idl_expression_value* get_value() const
	{
		return value_;
	}

	const std::vector<idl_expression*>& get_exprs() const
	{
		return exprs_;
	}

private:
	void set_op_link(Op op)
	{
		op_ = op;
	}

private: 
	Op op_ = None;
	idl_expression_value* value_ = nullptr;
	std::vector<idl_expression*> exprs_;
};
