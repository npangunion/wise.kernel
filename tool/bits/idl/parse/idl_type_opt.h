#pragma once

#include "idl_type_container.h"
#include <cstdlib>

class idl_type_opt : public idl_type
{
public: 
	enum class opt
	{
		opt_invalid,
		enable_cipher, 
		enable_checksum, 
		enable_sequence
	};

public:
	idl_type_opt()
	{
		type_ = type::option;
	}
	
	~idl_type_opt()
	{
	}

	void set_option(opt _opt)
	{
		opt_ = _opt;
	}

	opt get_option() const
	{
		return opt_;
	}

private:
	opt opt_;
};
