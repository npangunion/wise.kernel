#pragma once
#include <wise.kernel/net/protocol/bits/bits_packet.hpp>
#include <wise.kernel/net/protocol/bits/bits_packer.hpp>

#include <stdint.h>
#include <map>
#include <string>
#include <vector>

#include "game_topics..hpp"

namespace real {

enum class game_result
{
	success,
	fail_enter_timeout,
	fail_ready_timeout,
	fail_input_timeout,
	failed_with_user_left,
};


enum class rsp_kind
{
	none,
	rock,
	scissors,
	paper,
};


struct game_user_info
{
	std::wstring nickname;
	uint64_t uid = 0;
	uint32_t elo = 0;
};


class syn_enter_player : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 
	game_user_info user;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::play::category), 
			static_cast<wise::kernel::topic::group_t>(game::play::group), 
			static_cast<wise::kernel::topic::type_t>(game::play::syn_enter_player)
		);

		return topic_;
	}

	syn_enter_player()
	: wise::kernel::bits_packet(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack(user);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack(user);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( syn_enter_player )
};


class syn_leave_player : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 
	uint64_t uid = 0;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::play::category), 
			static_cast<wise::kernel::topic::group_t>(game::play::group), 
			static_cast<wise::kernel::topic::type_t>(game::play::syn_leave_player)
		);

		return topic_;
	}

	syn_leave_player()
	: wise::kernel::bits_packet(get_topic())
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

public: WISE_MESSAGE_DESC( syn_leave_player )
};


class syn_begin_ready : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::play::category), 
			static_cast<wise::kernel::topic::group_t>(game::play::group), 
			static_cast<wise::kernel::topic::type_t>(game::play::syn_begin_ready)
		);

		return topic_;
	}

	syn_begin_ready()
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

public: WISE_MESSAGE_DESC( syn_begin_ready )
};


class req_player_ready : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::play::category), 
			static_cast<wise::kernel::topic::group_t>(game::play::group), 
			static_cast<wise::kernel::topic::type_t>(game::play::req_player_ready)
		);

		return topic_;
	}

	req_player_ready()
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

public: WISE_MESSAGE_DESC( req_player_ready )
};


class res_player_ready : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::play::category), 
			static_cast<wise::kernel::topic::group_t>(game::play::group), 
			static_cast<wise::kernel::topic::type_t>(game::play::res_player_ready)
		);

		return topic_;
	}

	res_player_ready()
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

public: WISE_MESSAGE_DESC( res_player_ready )
};


class syn_start_play : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::play::category), 
			static_cast<wise::kernel::topic::group_t>(game::play::group), 
			static_cast<wise::kernel::topic::type_t>(game::play::syn_start_play)
		);

		return topic_;
	}

	syn_start_play()
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

public: WISE_MESSAGE_DESC( syn_start_play )
};


class req_input_rsp : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 
	rsp_kind rsp;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::play::category), 
			static_cast<wise::kernel::topic::group_t>(game::play::group), 
			static_cast<wise::kernel::topic::type_t>(game::play::req_input_rsp)
		);

		return topic_;
	}

	req_input_rsp()
	: wise::kernel::bits_packet(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack_enum(rsp);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack_enum(rsp);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( req_input_rsp )
};


class syn_rsp_result : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 
	game_result result;
	uint64_t winner = 0;
	rsp_kind me;
	rsp_kind other;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::play::category), 
			static_cast<wise::kernel::topic::group_t>(game::play::group), 
			static_cast<wise::kernel::topic::type_t>(game::play::syn_rsp_result)
		);

		return topic_;
	}

	syn_rsp_result()
	: wise::kernel::bits_packet(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack_enum(result);
		packer.pack(winner);
		packer.pack_enum(me);
		packer.pack_enum(other);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack_enum(result);
		packer.unpack(winner);
		packer.unpack_enum(me);
		packer.unpack_enum(other);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( syn_rsp_result )
};


class syn_game_end : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 
	game_result result;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::play::category), 
			static_cast<wise::kernel::topic::group_t>(game::play::group), 
			static_cast<wise::kernel::topic::type_t>(game::play::syn_game_end)
		);

		return topic_;
	}

	syn_game_end()
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

public: WISE_MESSAGE_DESC( syn_game_end )
};


class syn_game_invalid_state : public wise::kernel::bits_packet
{
	using super = wise::kernel::bits_packet;
public: // field variables 
	std::string reason;


public: // topic and constructor
	static const wise::kernel::topic& get_topic() 
	{
		static wise::kernel::topic topic_( 
			static_cast<wise::kernel::topic::category_t>(game::play::category), 
			static_cast<wise::kernel::topic::group_t>(game::play::group), 
			static_cast<wise::kernel::topic::type_t>(game::play::syn_game_invalid_state)
		);

		return topic_;
	}

	syn_game_invalid_state()
	: wise::kernel::bits_packet(get_topic())
	{
	}

public: // serialize
	bool pack(::wise::kernel::bits_packer& packer) override
	{
		super::pack(packer);
		packer.pack(reason);
		return packer.is_valid();
	}

	bool unpack(::wise::kernel::bits_packer& packer) override
	{
		super::unpack(packer);
		packer.unpack(reason);
		return packer.is_valid();
	}

public: WISE_MESSAGE_DESC( syn_game_invalid_state )
};


} // real

// struct serialization section
namespace wise { 
namespace kernel { 

// ::real::game_user_info serialization begin { 
template<> inline bool pack(bits_packer& packer, const ::real::game_user_info& tv) 
{
	packer.pack(tv.nickname);
	packer.pack(tv.uid);
	packer.pack(tv.elo);
	return packer.is_valid();
}

template<> inline bool unpack(bits_packer& packer, ::real::game_user_info& tv) 
{
	packer.unpack(tv.nickname);
	packer.unpack(tv.uid);
	packer.unpack(tv.elo);
	return packer.is_valid();
}
// } ::real::game_user_info serialization end

} // kernel
} // wise

