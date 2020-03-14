#include "stdafx.h"
#include "tx_test.hpp"
#include <wise/base/logger.hpp>

namespace query {

wise::tx::result simple_query::execute_query(dbc::statement::ptr stmt)
{
	stmt->bind( 0, &bind.result, dbc::statement::PARAM_OUT );
#if USE_NAME_FIELD
	stmt->bind_strings( 1, bind.name.c_str(), bind.name.length(), 1 );
#endif
	stmt->bind( 2, &bind.item1.id );
	stmt->bind( 3, &bind.item1.price );
	stmt->bind_bool( 4, bind.is_ok );

	if ( bind.options.size() < 5)
	{
		WISE_ERROR( "tx invalid vector size. tx : simple_query, name: options, count: 5" );
		return result::fail_invalid_vector_size;
	}
	stmt->bind( 5, &bind.options[0].add_hp );
	stmt->bind( 6, &bind.options[0].add_sp );
	stmt->bind( 7, &bind.options[1].add_hp );
	stmt->bind( 8, &bind.options[1].add_sp );
	stmt->bind( 9, &bind.options[2].add_hp );
	stmt->bind( 10, &bind.options[2].add_sp );
	stmt->bind( 11, &bind.options[3].add_hp );
	stmt->bind( 12, &bind.options[3].add_sp );
	stmt->bind( 13, &bind.options[4].add_hp );
	stmt->bind( 14, &bind.options[4].add_sp );

	if ( bind.items.size() < 10)
	{
		WISE_ERROR( "tx invalid vector size. tx : simple_query, name: items, count: 10" );
		return result::fail_invalid_vector_size;
	}
	stmt->bind( 15, &bind.items[0] );
	stmt->bind( 16, &bind.items[1] );
	stmt->bind( 17, &bind.items[2] );
	stmt->bind( 18, &bind.items[3] );
	stmt->bind( 19, &bind.items[4] );
	stmt->bind( 20, &bind.items[5] );
	stmt->bind( 21, &bind.items[6] );
	stmt->bind( 22, &bind.items[7] );
	stmt->bind( 23, &bind.items[8] );
	stmt->bind( 24, &bind.items[9] );

	stmt->execute();

	// rs 1
	{
		auto& rs = stmt->next_result();

		if ( rs.next() )
		{
#if USE_ID_FIELD
			rs.get_ref( 0, rs_1.id );
#endif
			rs.get_ref( 1, rs_1.sname );
		}
	}
	// rs 2
	{
		auto& rs = stmt->next_result();

		while ( rs.next() )
		{
			rs_2_t rset;
			rset.id = rs.get<uint8_t>( 0 );
			rset.fv = rs.get<float>( 1 );
			rs_2.push_back(rset);
		}
	}

	return result::success;
}

bool simple_query::pack(wise::zen_packer& packer)
{
	wise::tx::pack(packer);
	bind.pack(packer);
	// result sets: 
	rs_1.pack(packer);
	uint16_t len = rs_2.size();
	packer.pack(len);
	for ( auto& rs : rs_2) 
	{
		rs_2.pack(packer); 
	}
	return packer.is_valid();
}

bool simple_query::unpack(wise::zen_packer& packer)
{
	wise::tx::unpack(packer);
	bind.unpack(packer);
	// result sets: 
	rs_1.unpack(packer);
	uint16_t len = 0
	packer.unpack(len);
	for ( uint16_t i=0; i < len; ++i) 
	{
		rs_2_t rs; 
		rs.unpack(packer);
		rs_2.push_back(rs);
	}
	return packer.is_valid();
}

bool simple_query::bind_t::pack(wise::zen_packer& packer)
{
	packer.pack(result);
#if USE_NAME_FIELD
	packer.pack(name);
#endif
	packer.pack(item1);
	packer.pack(is_ok);
	packer.pack(options);
	packer.pack(items);
	return packer.is_valid();
}

bool simple_query::bind_t::unpack(wise::zen_packer& packer)
{
	packer.unpack(result);
#if USE_NAME_FIELD
	packer.unpack(name);
#endif
	packer.unpack(item1);
	packer.unpack(is_ok);
	packer.unpack(options);
	packer.unpack(items);
	return packer.is_valid();
}

bool simple_query::rs_1_t::pack(wise::zen_packer& packer)
{
#if USE_ID_FIELD
	packer.pack(id);
#endif
	packer.pack(sname);
	return packer.is_valid();
}

bool simple_query::rs_1_t::unpack(wise::zen_packer& packer)
{
#if USE_ID_FIELD
	packer.unpack(id);
#endif
	packer.unpack(sname);
	return packer.is_valid();
}
bool simple_query::rs_2_t::pack(wise::zen_packer& packer)
{
	packer.pack(id);
	packer.pack(fv);
	return packer.is_valid();
}

bool simple_query::rs_2_t::unpack(wise::zen_packer& packer)
{
	packer.unpack(id);
	packer.unpack(fv);
	return packer.is_valid();
}

} // query
