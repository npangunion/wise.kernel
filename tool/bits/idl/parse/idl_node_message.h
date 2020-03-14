#pragma once 

#include "idl_node_struct.h"
#include "idl_type_opt.h"
#include <vector>

class idl_node_message : public idl_node_struct
{
public:
	idl_node_message()
	{
		type_ = Message;
	}

	~idl_node_message()
	{
	}

	void set_super_class(idl_type* type)
	{
		super_class_ = type;
	}

	idl_type* get_super_class() const
	{
		return super_class_;
	}

	bool has_hx() const
	{
		auto sc = get_super_class();

		if (sc)
		{
			auto scft = static_cast<const idl_type_full*>(sc);
			return scft->get_name() == "hx";
		}

		return false;
	}

	bool is_set_enable_cipher() const;

	bool is_set_enable_sequence() const;

	bool is_set_enable_checksum() const;

	bool is_set(idl_type_opt::opt option) const;

private: 
	idl_type* super_class_ = nullptr;
};

