#pragma once
#include <wise.kernel/net/protocol/bits/bits_packet.hpp>
#include <wise.kernel/net/protocol/bits/bits_packer.hpp>

#include <stdint.h>
#include <map>
#include <string>
#include <vector>

#include "game_topics..hpp"

namespace real {

enum class scene_result
{
	success,
	fail_timeout,
	fail_create_scene,
	fail_scene_not_found,
	fail_scene_handler_not_found,
	fail_scene_user_in_invalid_state,
	fail_scene_user_not_found,
};


struct scene_user_info
{
	std::wstring nickname;
	uint64_t uid = 0;
	uint64_t did = 0;
	uint64_t did_character = 0;
	uint32_t elo = 0;
};


class req_enter_scene : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 
	scene_user_info me;
	uint64_t scene_id = 0;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::scene_service::category), 
			static_cast<wise::kernel::topic::group_t>(game::scene_service::group), 
			static_cast<wise::kernel::topic::type_t>(game::scene_service::req_enter_scene)
		);

		return topic_;
	}

	req_enter_scene()
	: wise::kernel::bits_packet(get_topic())
	{
		enable_cipher = true;
		enable_checksum = true;
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack(me);
		packer.pack(scene_id);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack(me);
		packer.unpack(scene_id);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( req_enter_scene )
};


class res_enter_scene : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 
	scene_result result;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::scene_service::category), 
			static_cast<wise::kernel::topic::group_t>(game::scene_service::group), 
			static_cast<wise::kernel::topic::type_t>(game::scene_service::res_enter_scene)
		);

		return topic_;
	}

	res_enter_scene()
	: wise::kernel::bits_packet(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack_enum(result);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack_enum(result);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( res_enter_scene )
};


class req_reenter_scene : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 
	uint64_t scene_id = 0;
	uint64_t uid = 0;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::scene_service::category), 
			static_cast<wise::kernel::topic::group_t>(game::scene_service::group), 
			static_cast<wise::kernel::topic::type_t>(game::scene_service::req_reenter_scene)
		);

		return topic_;
	}

	req_reenter_scene()
	: wise::kernel::bits_packet(get_topic())
	{
		enable_cipher = true;
		enable_checksum = true;
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack(scene_id);
		packer.pack(uid);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack(scene_id);
		packer.unpack(uid);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( req_reenter_scene )
};


class res_reenter_scene : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 
	scene_result result;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::scene_service::category), 
			static_cast<wise::kernel::topic::group_t>(game::scene_service::group), 
			static_cast<wise::kernel::topic::type_t>(game::scene_service::res_reenter_scene)
		);

		return topic_;
	}

	res_reenter_scene()
	: wise::kernel::bits_packet(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack_enum(result);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack_enum(result);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( res_reenter_scene )
};


class req_leave_scene : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::scene_service::category), 
			static_cast<wise::kernel::topic::group_t>(game::scene_service::group), 
			static_cast<wise::kernel::topic::type_t>(game::scene_service::req_leave_scene)
		);

		return topic_;
	}

	req_leave_scene()
	: wise::kernel::bits_packet(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( req_leave_scene )
};


class res_leave_scene : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 
	scene_result result;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::scene_service::category), 
			static_cast<wise::kernel::topic::group_t>(game::scene_service::group), 
			static_cast<wise::kernel::topic::type_t>(game::scene_service::res_leave_scene)
		);

		return topic_;
	}

	res_leave_scene()
	: wise::kernel::bits_packet(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack_enum(result);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack_enum(result);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( res_leave_scene )
};


class syn_master_scene_handler : public wise::hx
{
	using super = wise::hx;
public: // field variables 
	uint32_t domain;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::scene_master::category), 
			static_cast<wise::kernel::topic::group_t>(game::scene_master::group), 
			static_cast<wise::kernel::topic::type_t>(game::scene_master::syn_master_scene_handler)
		);

		return topic_;
	}

	syn_master_scene_handler()
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

public: WISE_MESSAGE_DESC( syn_master_scene_handler )
};


class req_ss_create_scene : public wise::hx
{
	using super = wise::hx;
public: // field variables 
	uint64_t scene_id = 0;
	uint64_t uid_1 = 0;
	uint64_t uid_2 = 0;
	std::string options;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::scene_master::category), 
			static_cast<wise::kernel::topic::group_t>(game::scene_master::group), 
			static_cast<wise::kernel::topic::type_t>(game::scene_master::req_ss_create_scene)
		);

		return topic_;
	}

	req_ss_create_scene()
	: wise::hx(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack(scene_id);
		packer.pack(uid_1);
		packer.pack(uid_2);
		packer.pack(options);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack(scene_id);
		packer.unpack(uid_1);
		packer.unpack(uid_2);
		packer.unpack(options);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( req_ss_create_scene )
};


class res_ss_create_scene : public wise::hx
{
	using super = wise::hx;
public: // field variables 
	scene_result result;
	uint64_t scene_id = 0;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::scene_master::category), 
			static_cast<wise::kernel::topic::group_t>(game::scene_master::group), 
			static_cast<wise::kernel::topic::type_t>(game::scene_master::res_ss_create_scene)
		);

		return topic_;
	}

	res_ss_create_scene()
	: wise::hx(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack_enum(result);
		packer.pack(scene_id);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack_enum(result);
		packer.unpack(scene_id);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( res_ss_create_scene )
};


class req_ss_enter_scene : public wise::hx
{
	using super = wise::hx;
public: // field variables 
	uint64_t scene_id = 0;
	scene_user_info me;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::scene_master::category), 
			static_cast<wise::kernel::topic::group_t>(game::scene_master::group), 
			static_cast<wise::kernel::topic::type_t>(game::scene_master::req_ss_enter_scene)
		);

		return topic_;
	}

	req_ss_enter_scene()
	: wise::hx(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack(scene_id);
		packer.pack(me);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack(scene_id);
		packer.unpack(me);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( req_ss_enter_scene )
};


class res_ss_enter_scene : public wise::hx
{
	using super = wise::hx;
public: // field variables 
	scene_result result;
	uint64_t scene_id = 0;
	uint64_t uid;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::scene_master::category), 
			static_cast<wise::kernel::topic::group_t>(game::scene_master::group), 
			static_cast<wise::kernel::topic::type_t>(game::scene_master::res_ss_enter_scene)
		);

		return topic_;
	}

	res_ss_enter_scene()
	: wise::hx(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack_enum(result);
		packer.pack(scene_id);
		packer.pack(uid);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack_enum(result);
		packer.unpack(scene_id);
		packer.unpack(uid);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( res_ss_enter_scene )
};


class req_ss_leave_scene : public wise::hx
{
	using super = wise::hx;
public: // field variables 
	uint64_t scene_id = 0;
	uint64_t uid = 0;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::scene_master::category), 
			static_cast<wise::kernel::topic::group_t>(game::scene_master::group), 
			static_cast<wise::kernel::topic::type_t>(game::scene_master::req_ss_leave_scene)
		);

		return topic_;
	}

	req_ss_leave_scene()
	: wise::hx(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack(scene_id);
		packer.pack(uid);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack(scene_id);
		packer.unpack(uid);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( req_ss_leave_scene )
};


class res_ss_leave_scene : public wise::hx
{
	using super = wise::hx;
public: // field variables 
	scene_result result;
	uint64_t scene_id = 0;
	uint64_t uid;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::scene_master::category), 
			static_cast<wise::kernel::topic::group_t>(game::scene_master::group), 
			static_cast<wise::kernel::topic::type_t>(game::scene_master::res_ss_leave_scene)
		);

		return topic_;
	}

	res_ss_leave_scene()
	: wise::hx(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack_enum(result);
		packer.pack(scene_id);
		packer.pack(uid);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack_enum(result);
		packer.unpack(scene_id);
		packer.unpack(uid);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( res_ss_leave_scene )
};


class req_ss_dispose_scene : public wise::hx
{
	using super = wise::hx;
public: // field variables 
	uint64_t scene_id = 0;
	std::string reason;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::scene_master::category), 
			static_cast<wise::kernel::topic::group_t>(game::scene_master::group), 
			static_cast<wise::kernel::topic::type_t>(game::scene_master::req_ss_dispose_scene)
		);

		return topic_;
	}

	req_ss_dispose_scene()
	: wise::hx(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack(scene_id);
		packer.pack(reason);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack(scene_id);
		packer.unpack(reason);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( req_ss_dispose_scene )
};


class res_ss_dispose_scene : public wise::hx
{
	using super = wise::hx;
public: // field variables 
	scene_result result;
	uint64_t scene_id = 0;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::scene_master::category), 
			static_cast<wise::kernel::topic::group_t>(game::scene_master::group), 
			static_cast<wise::kernel::topic::type_t>(game::scene_master::res_ss_dispose_scene)
		);

		return topic_;
	}

	res_ss_dispose_scene()
	: wise::hx(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack_enum(result);
		packer.pack(scene_id);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack_enum(result);
		packer.unpack(scene_id);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( res_ss_dispose_scene )
};


} // real

// struct serialization section
namespace wise { 
namespace kernel { 

// ::real::scene_user_info serialization begin { 
template<> inline bool pack(bits_packer& packer, const ::real::scene_user_info& tv) 
{
	packer.pack(tv.nickname);
	packer.pack(tv.uid);
	packer.pack(tv.did);
	packer.pack(tv.did_character);
	packer.pack(tv.elo);
	return packer.is_valid();
}

template<> inline bool unpack(bits_packer& packer, ::real::scene_user_info& tv) 
{
	packer.unpack(tv.nickname);
	packer.unpack(tv.uid);
	packer.unpack(tv.did);
	packer.unpack(tv.did_character);
	packer.unpack(tv.elo);
	return packer.is_valid();
}
// } ::real::scene_user_info serialization end

} // kernel
} // wise

