#pragma once
#include <wise/net/protocol/zen/zen_message.hpp>
#include <wise/net/protocol/zen/zen_packer.hpp>

#include <stdint.h>
#include <map>
#include <string>
#include <vector>

#include "sample2.hpp"
#include "simple_topic.hpp"
#include "shop_topic.hpp"

namespace shop {

enum class types
{
	v1,
	v2 = 1,
	v3,
};


struct item
{
	uint32_t id = 0;
#if _DEBUG
	int32_t v = 5;
#else 
	double v = 5;
#endif
	float key = 0.f;
	std::string name;
	std::wstring desc;
	common::hello he;
	common::TestEnum te;
};


class req_buy_item : public wise::zen_message
{
	using super = wise::zen_message;
public: // field variables 
	std::vector<item> items;
#if _DEBUG
	types v1;
#endif


public: // topic and constructor
	static const wise::topic& get_topic() 
	{
		static wise::topic topic_( 
			static_cast<wise::topic::category_t>(play::shop::category), 
			static_cast<wise::topic::group_t>(play::shop::group), 
			static_cast<wise::topic::type_t>(play::shop::req_buy_item)
		);

		return topic_;
	}

	req_buy_item()
	: wise::zen_message(get_topic())
	{
		enable_sequence = true;
	}

public: // serialize
	bool pack(::wise::zen_packer& packer) override
	{
		super::pack(packer);
		packer.pack(items);
#if _DEBUG
		packer.pack_enum(v1);
#endif
		return packer.is_valid();
	}

	bool unpack(::wise::zen_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack(items);
#if _DEBUG
		packer.unpack_enum(v1);
#endif
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( req_buy_item )
};


class hx_example_with_options : public wise::hx
{
	using super = wise::hx;
public: // field variables 
	std::vector<item> items;


public: // topic and constructor
	static const wise::topic& get_topic() 
	{
		static wise::topic topic_( 
			static_cast<wise::topic::category_t>(play::shop::category), 
			static_cast<wise::topic::group_t>(play::shop::group), 
			static_cast<wise::topic::type_t>(play::shop::hx_example)
		);

		return topic_;
	}

	hx_example_with_options()
	: wise::hx(get_topic())
	{
		enable_cipher = true;
		enable_checksum = true;
		enable_sequence = true;
	}

public: // serialize
	bool pack(::wise::zen_packer& packer) override
	{
		super::pack(packer);
		packer.pack(items);
		return packer.is_valid();
	}

	bool unpack(::wise::zen_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack(items);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( hx_example_with_options )
};


} // shop

// struct serialization section
namespace wise {

// ::shop::item serialization begin { 
template<> inline bool pack(zen_packer& packer, const ::shop::item& tv) 
{
	packer.pack(tv.id);
#if _DEBUG
	packer.pack(tv.v);
#else 
	packer.pack(tv.v);
#endif
	packer.pack(tv.key);
	packer.pack(tv.name);
	packer.pack(tv.desc);
	packer.pack(tv.he);
	packer.pack_enum(tv.te);
	return packer.is_valid();
}

template<> inline bool unpack(zen_packer& packer, ::shop::item& tv) 
{
	packer.unpack(tv.id);
#if _DEBUG
	packer.unpack(tv.v);
#else 
	packer.unpack(tv.v);
#endif
	packer.unpack(tv.key);
	packer.unpack(tv.name);
	packer.unpack(tv.desc);
	packer.unpack(tv.he);
	packer.unpack_enum(tv.te);
	return packer.is_valid();
}
// } ::shop::item serialization end

} // wise

