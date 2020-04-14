#pragma once
#include <wise.kernel/net/protocol/bits/bits_packet.hpp>
#include <wise.kernel/net/protocol/bits/bits_packer.hpp>

#include <stdint.h>
#include <map>
#include <string>
#include <vector>

namespace cluster {

enum class topics
{
	category = 2,
	group = 2,
	syn_peer_up = 1,
	syn_peer_down,
	syn_actor_up,
	syn_actor_down,
	syn_client_up,
	syn_client_down,
};


} // cluster


