#pragma once
#include <wise.kernel/net/protocol/bits/bits_packet.hpp>
#include <wise.kernel/net/protocol/bits/bits_packer.hpp>

#include <stdint.h>
#include <map>
#include <string>
#include <vector>

#include "game_topics..hpp"

namespace real {

enum class match_result
{
	success,
	fail_invalid_state,
	fail_match_handler_not_found,
	fail_scene_handler_not_found,
	fail_scene_service_not_found,
	fail_timeout,
	fail_create_scene,
};


enum class match_state
{
	match_started,
	match_waiting_scene_create,
	match_waiting_scene_join,
};


struct match_user_info
{
	std::wstring nickname;
	uint64_t uid = 0;
	uint64_t did = 0;
	uint32_t elo = 0;
};


class req_match : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 
	uint32_t domain = 0;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::user::category), 
			static_cast<wise::kernel::topic::group_t>(game::user::group), 
			static_cast<wise::kernel::topic::type_t>(game::user::req_match)
		);

		return topic_;
	}

	req_match()
	: wise::kernel::bits_packet(get_topic())
	{
		enable_cipher = true;
		enable_checksum = true;
		enable_sequence = true;
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack(domain);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack(domain);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( req_match )
};


class res_match : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 
	match_result result;
	match_user_info peer;
	uint64_t scene_id = 0;
	std::string addr;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::user::category), 
			static_cast<wise::kernel::topic::group_t>(game::user::group), 
			static_cast<wise::kernel::topic::type_t>(game::user::res_match)
		);

		return topic_;
	}

	res_match()
	: wise::kernel::bits_packet(get_topic())
	{
		enable_cipher = true;
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack_enum(result);
		packer.pack(peer);
		packer.pack(scene_id);
		packer.pack(addr);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack_enum(result);
		packer.unpack(peer);
		packer.unpack(scene_id);
		packer.unpack(addr);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( res_match )
};


class syn_match_state : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 
	match_state state;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::user::category), 
			static_cast<wise::kernel::topic::group_t>(game::user::group), 
			static_cast<wise::kernel::topic::type_t>(game::user::syn_match_state)
		);

		return topic_;
	}

	syn_match_state()
	: wise::kernel::bits_packet(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack_enum(state);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack_enum(state);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( syn_match_state )
};


class syn_master_match_handler : public wise::hx
{
	using super = wise::hx;
public: // field variables 
	uint32_t domain;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::match_master::category), 
			static_cast<wise::kernel::topic::group_t>(game::match_master::group), 
			static_cast<wise::kernel::topic::type_t>(game::match_master::syn_master_match_handler)
		);

		return topic_;
	}

	syn_master_match_handler()
	: wise::hx(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack(domain);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack(domain);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( syn_master_match_handler )
};


class req_master_match : public wise::hx
{
	using super = wise::hx;
public: // field variables 
	uint32_t domain;
	match_user_info user;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::match_master::category), 
			static_cast<wise::kernel::topic::group_t>(game::match_master::group), 
			static_cast<wise::kernel::topic::type_t>(game::match_master::req_master_match)
		);

		return topic_;
	}

	req_master_match()
	: wise::hx(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack(domain);
		packer.pack(user);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack(domain);
		packer.unpack(user);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( req_master_match )
};


class res_master_match : public wise::hx
{
	using super = wise::hx;
public: // field variables 
	match_result result;
	match_user_info me;
	match_user_info peer;
	uint64_t scene_id = 0;
	std::string addr;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::match_master::category), 
			static_cast<wise::kernel::topic::group_t>(game::match_master::group), 
			static_cast<wise::kernel::topic::type_t>(game::match_master::res_master_match)
		);

		return topic_;
	}

	res_master_match()
	: wise::hx(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack_enum(result);
		packer.pack(me);
		packer.pack(peer);
		packer.pack(scene_id);
		packer.pack(addr);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack_enum(result);
		packer.unpack(me);
		packer.unpack(peer);
		packer.unpack(scene_id);
		packer.unpack(addr);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( res_master_match )
};


class req_master_cancel_match : public wise::hx
{
	using super = wise::hx;
public: // field variables 
	uint64_t uid = 0;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::match_master::category), 
			static_cast<wise::kernel::topic::group_t>(game::match_master::group), 
			static_cast<wise::kernel::topic::type_t>(game::match_master::req_master_cancel_match)
		);

		return topic_;
	}

	req_master_cancel_match()
	: wise::hx(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack(uid);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack(uid);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( req_master_cancel_match )
};


class res_master_cancel_match : public wise::hx
{
	using super = wise::hx;
public: // field variables 
	uint64_t uid = 0;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::match_master::category), 
			static_cast<wise::kernel::topic::group_t>(game::match_master::group), 
			static_cast<wise::kernel::topic::type_t>(game::match_master::res_master_cancel_match)
		);

		return topic_;
	}

	res_master_cancel_match()
	: wise::hx(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack(uid);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack(uid);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( res_master_cancel_match )
};


class req_master_create_scene : public wise::hx
{
	using super = wise::hx;
public: // field variables 
	match_user_info user_1;
	match_user_info user_2;
	std::string options;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::match_master::category), 
			static_cast<wise::kernel::topic::group_t>(game::match_master::group), 
			static_cast<wise::kernel::topic::type_t>(game::match_master::req_master_create_scene)
		);

		return topic_;
	}

	req_master_create_scene()
	: wise::hx(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack(user_1);
		packer.pack(user_2);
		packer.pack(options);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack(user_1);
		packer.unpack(user_2);
		packer.unpack(options);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( req_master_create_scene )
};


class res_master_create_scene : public wise::hx
{
	using super = wise::hx;
public: // field variables 
	match_result result;
	uint64_t uid_1;
	uint64_t uid_2;
	uint64_t scene_id;
	std::string addr;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::match_master::category), 
			static_cast<wise::kernel::topic::group_t>(game::match_master::group), 
			static_cast<wise::kernel::topic::type_t>(game::match_master::res_master_create_scene)
		);

		return topic_;
	}

	res_master_create_scene()
	: wise::hx(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack_enum(result);
		packer.pack(uid_1);
		packer.pack(uid_2);
		packer.pack(scene_id);
		packer.pack(addr);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack_enum(result);
		packer.unpack(uid_1);
		packer.unpack(uid_2);
		packer.unpack(scene_id);
		packer.unpack(addr);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( res_master_create_scene )
};


class syn_master_match_state : public wise::hx
{
	using super = wise::hx;
public: // field variables 
	match_state state;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::match_master::category), 
			static_cast<wise::kernel::topic::group_t>(game::match_master::group), 
			static_cast<wise::kernel::topic::type_t>(game::match_master::syn_master_match_state)
		);

		return topic_;
	}

	syn_master_match_state()
	: wise::hx(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack_enum(state);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack_enum(state);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( syn_master_match_state )
};


} // real

// struct serialization section
namespace wise { 
namespace kernel { 

// ::real::match_user_info serialization begin { 
template<> inline bool pack(bits_packer& packer, const ::real::match_user_info& tv) 
{
	packer.pack(tv.nickname);
	packer.pack(tv.uid);
	packer.pack(tv.did);
	packer.pack(tv.elo);
	return packer.is_valid();
}

template<> inline bool unpack(bits_packer& packer, ::real::match_user_info& tv) 
{
	packer.unpack(tv.nickname);
	packer.unpack(tv.uid);
	packer.unpack(tv.did);
	packer.unpack(tv.elo);
	return packer.is_valid();
}
// } ::real::match_user_info serialization end

} // kernel
} // wise

