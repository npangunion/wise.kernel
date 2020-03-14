#pragma once 

#include <string>

class tx_query
{
public:
	tx_query()
	{
	}

	~tx_query()
	{
	}

	void set_query(const char* q)
	{
		query_ = q;
	}

	const std::string& get_query() const
	{
		return query_;
	}

private: 
	std::string query_;
};