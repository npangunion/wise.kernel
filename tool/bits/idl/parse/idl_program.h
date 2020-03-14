#pragma once 

#include "idl_node.h"
#include "idl_node_message.h"
#include <memory>
#include <string>
#include <vector>

class idl_program
{
public: 
	using ptr = std::shared_ptr<idl_program>;

public:
	idl_program(const std::string& path)
		: path_(path)
	{}

	~idl_program()
	{
		for (auto& np : nodes_)
		{
			delete np;
		}

		nodes_.clear();
	}

	void add_node(idl_node* node)
	{
		WISE_EXPECT(node);

		nodes_.push_back(node);

		check_node_type(node);
	}

	const std::string& get_path() const
	{
		return path_;
	}

	const std::vector<idl_node*>& get_nodes() const
	{
		return nodes_;
	}

	const idl_node* get_namespace() const
	{
		for (auto& n : nodes_)
		{
			if (n->get_type() == idl_node::Namespace)
			{
				return n;
			}
		}

		return nullptr;
	}

	const std::string get_fullname(const std::string& name)
	{
		auto ns = get_namespace();

		if (ns)
		{
			return ns->get_name() + /* "::" + */ name;
		}
		
		return name;
	}

	bool has_message() const
	{
		return has_message_;
	}

	bool has_struct() const
	{
		return has_struct_;
	}

	bool has_tx() const
	{
		return has_tx_;
	}

	bool has_hx() const
	{
		return has_hx_;
	}

private: 
	void check_node_type(idl_node* node)
	{
		switch (node->get_type())
		{
		case idl_node::Type::Message:
			has_message_ = true;
			check_hx(node);
			break;
		case idl_node::Type::Struct:
			has_struct_ = true;
			break;
		case idl_node::Type::Tx:
			has_tx_ = true;
			break;
		}
	}

	void check_hx(idl_node* node)
	{
		auto nm = static_cast<const idl_node_message*>(node);
		has_hx_ = has_hx_ || nm->has_hx();
	}

private: 
	std::string path_;
	std::vector<idl_node*> nodes_;
	bool has_message_ = false;
	bool has_struct_ = false;
	bool has_tx_ = false;
	bool has_hx_ = false;
};
