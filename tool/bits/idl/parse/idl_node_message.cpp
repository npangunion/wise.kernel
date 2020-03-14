#include "stdafx.h"
#include "idl_node_message.h"
#include "idl_type_opt.h"

bool idl_node_message::is_set_enable_cipher() const
{
	return is_set(idl_type_opt::opt::enable_cipher);
}

bool idl_node_message::is_set_enable_sequence() const
{
	return is_set(idl_type_opt::opt::enable_sequence);
}

bool idl_node_message::is_set_enable_checksum() const
{
	return is_set(idl_type_opt::opt::enable_checksum);
}

bool idl_node_message::is_set(idl_type_opt::opt option) const
{
	auto& flds = get_fields();

	for (auto& f : flds)
	{
		auto ft = f->get_type();

		if (ft->get_type() == idl_type::option)
		{
			auto opt = static_cast<const idl_type_opt*>(ft);

			if (opt->get_option() == option)
			{
				return true;
			}
		}
	}

	return false;
}
