#pragma once 

#include "idl_node.h"

#include <sstream>
#include <vector>

class idl_node_namespace : public idl_node
{
public:
	idl_node_namespace()
		: idl_node()
	{
		type_ = Type::Namespace;
	}

	~idl_node_namespace()
	{
	}

	void add_namespace(const std::string& ns)
	{
		ns_.push_back(ns);

		std::ostringstream oss;

		for (auto& nn : ns_)
		{
			oss << nn << "::";
		}

		set_name(oss.str());
	}

	const std::vector<std::string>& get_namespace() const
	{
		return ns_;
	}

private:
	std::vector<std::string> ns_;
};

