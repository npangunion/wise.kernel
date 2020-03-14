#pragma once
#include <wise/net/protocol/zen/zen_message.hpp>
#include <wise/net/protocol/zen/zen_packer.hpp>

#include <wise/service/db/tx.hpp>

#include <stdint.h>
#include <map>
#include <string>
#include <vector>

#include "query_topic.hpp"

namespace query {

struct option
{
	uint32_t add_hp;
	uint32_t add_sp;
};


struct item
{
	uint64_t id;
	uint32_t price;
};


class simple_query : public wise::tx
{
public: // query, topic, constructor
	static constexpr wchar_t* query = L"{ CALL Usp.Hello(?, ?, ?, ?, ?, ?, ?, ?) }";

	static const wise::topic& get_topic() 
	{
		static wise::topic topic_( 
			static_cast<wise::topic::category_t>(db::query::category), 
			static_cast<wise::topic::group_t>(db::query::group), 
			static_cast<wise::topic::type_t>(db::query::simple)
		);

		return topic_;
	}

	simple_query()
	: wise::tx(get_topic())
	{
		query_ = query;
	}

	result execute_query(dbc::statement::ptr stmt) override;

public: // bind structure
	struct bind_t
	{
		uint32_t result;
#if USE_NAME_FIELD
		std::string name;
#endif
		item item1;
		bool is_ok;
		std::vector<option> options;
		std::vector<uint32_t> items;

		bool pack(wise::zen_packer& packer); 
		bool unpack(wise::zen_packer& packer); 
	} bind;

public: // result set structure 

	struct rs_1_t
	{
#if USE_ID_FIELD
		uint32_t id;
#endif
		std::wstring sname;

		bool pack(wise::zen_packer& packer); 
		bool unpack(wise::zen_packer& packer); 
	};
	rs_1_t rs_1;

	struct rs_2_t
	{
		uint8_t id;
		float fv;

		bool pack(wise::zen_packer& packer); 
		bool unpack(wise::zen_packer& packer); 
	};
	std::vector<rs_2_t> rs_2;

public: // tx serialization
	bool pack(wise::zen_packer& packer) override;
	bool unpack(wise::zen_packer& packer) override;

public: WISE_MESSAGE_DESC( simple_query )
};


} // query

// struct serialization section
namespace wise {

// ::query::option serialization begin { 
template<> inline bool pack(zen_packer& packer, const ::query::option& tv) 
{
	packer.pack(tv.add_hp);
	packer.pack(tv.add_sp);
	return packer.is_valid();
}

template<> inline bool unpack(zen_packer& packer, ::query::option& tv) 
{
	packer.unpack(tv.add_hp);
	packer.unpack(tv.add_sp);
	return packer.is_valid();
}
// } ::query::option serialization end

// ::query::item serialization begin { 
template<> inline bool pack(zen_packer& packer, const ::query::item& tv) 
{
	packer.pack(tv.id);
	packer.pack(tv.price);
	return packer.is_valid();
}

template<> inline bool unpack(zen_packer& packer, ::query::item& tv) 
{
	packer.unpack(tv.id);
	packer.unpack(tv.price);
	return packer.is_valid();
}
// } ::query::item serialization end

} // wise

