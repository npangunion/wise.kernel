#pragma once 

#include "tx_result_set.h"
#include <vector>

class tx_result
{
public:
	tx_result()
	{
	}

	~tx_result()
	{

	}

	void add_result_set(tx_result_set* rs)
	{
		WISE_ASSERT(rs != nullptr);

		results_.push_back(rs);
	}

	const std::vector<tx_result_set*> get_results() const
	{
		return results_;
	}

private: 
	std::vector<tx_result_set*> results_;

};