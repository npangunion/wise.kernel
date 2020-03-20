#pragma once 

#include "idl_node.h"
#include "tx/tx_topic.h"
#include "tx/tx_bind.h"
#include "tx/tx_result.h"
#include "tx/tx_result_set.h"
#include "tx/tx_query.h"

#include <wise.kernel/core/macros.hpp>
#include <sstream>
#include <vector>

struct tx_gen_context
{
	std::string name;
	int bind_seq = 0;
	std::string query;
};

class idl_node_tx: public idl_node
{
public:
	idl_node_tx()
		: idl_node()
	{
		type_ = Tx;
	}

	~idl_node_tx()
	{
	}

	void set_topic(tx_topic* topic)
	{
		WISE_ASSERT(topic_ == nullptr);
		WISE_ASSERT(topic != nullptr);

		topic_ = topic;
	}

	void set_bind(tx_bind* bind)
	{
		WISE_ASSERT(bind_ == nullptr);
		WISE_ASSERT(bind != nullptr);

		bind_ = bind;
	}

	void set_result(tx_result* result)
	{
		WISE_ASSERT(result_ == nullptr);
		WISE_ASSERT(result != nullptr);

		result_ = result;
	}

	void set_query(tx_query* query)
	{
		WISE_ASSERT(query_ == nullptr);
		WISE_ASSERT(query != nullptr);

		query_ = query;
	}

	const tx_topic* get_topic() const 
	{
		return topic_;
	}

	const tx_bind* get_bind() const
	{
		return bind_;
	}

	const tx_result* get_result() const
	{
		return result_;
	}

	const tx_query* get_query() const
	{
		return query_;
	}

private: 
	tx_topic* topic_ = nullptr;
	tx_bind* bind_ = nullptr;
	tx_result* result_ = nullptr;
	tx_query* query_ = nullptr;
};

