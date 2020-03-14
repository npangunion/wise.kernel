#pragma once

#include "idl_type_container.h"
#include <cstdlib>

class idl_type_macro : public idl_type
{
public:
	idl_type_macro()
	{
		type_ = type::macro;
	}
	
	~idl_type_macro()
	{
	}

	void set_line(const char* line)
	{
		// TODO: free line? (strdup로 생성됨. 테스트 후 정리)

		line_ = line;

		std::ostringstream oss;

		oss << "macro: " << line_;

		set_name(oss.str());
	}

	const std::string& get_line() const
	{
		return line_;
	}

private:
	std::string line_;
};
