#pragma once 

#include "idl_node.h"
#include <wise/base/macros.hpp>
#include <sstream>
#include <vector>

class idl_node_include : public idl_node
{
public:
	idl_node_include()
		: idl_node()
	{
		type_ = Include;
	}

	~idl_node_include()
	{
	}

	void add_path(const std::string& path)
	{
		WISE_EXPECT(!path.empty());

		path_.push_back(path);

		set_name(get_path_string());
	}

	const std::vector<std::string>& get_path() const
	{
		return path_;
	}

	const std::string get_path_string() const
	{
		std::stringstream os;

		for (size_t i=0; i<path_.size(); ++i)
		{
			if (i > 0) os << "/";

			os << path_[i];
		}

		return os.str();
	}

private:
	std::vector<std::string> path_;
};

