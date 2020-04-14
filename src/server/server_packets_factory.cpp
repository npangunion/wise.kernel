#include "pch.hpp"
#include <wise.kernel/net/protocol/bits/bits_factory.hpp>
#include <wise.kernel/core/mem_tracker.hpp>
#include <wise.kernel/server/server_packets.hpp>

#define WISE_ADD_BITS(cls) wise::kernel::bits_factory::inst().add( cls::get_topic(), []() { return wise_shared<cls>(); } );

void add_server_packets()
{
	WISE_ADD_BITS(::cluster::messages::syn_peer_up);
	WISE_ADD_BITS(::cluster::messages::syn_peer_down);
	WISE_ADD_BITS(::cluster::messages::syn_actor_up);
	WISE_ADD_BITS(::cluster::messages::syn_actor_down);
}
