#pragma once 

#include "../idl_type_topic.h"

class tx_topic
{
public:
	tx_topic()
	{
	}

	~tx_topic()
	{
	}

	void set_topic(idl_type_topic* topic)
	{
		WISE_ASSERT(topic_ == nullptr);
		WISE_ASSERT(topic != nullptr);
		
		topic_ = topic;
	}

	const idl_type_topic* get_topic() const 
	{
		return topic_;
	}

private: 
	idl_type_topic* topic_ = nullptr;
};