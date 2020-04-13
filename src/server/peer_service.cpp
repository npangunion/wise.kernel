#include <pch.hpp>
#include <wise.kernel/server/peer_service.hpp>

namespace wise {
namespace kernel {

bool peer_service::setup(const nlohmann::json& _json)
{
	// listen address
	// peers
	// - address 
	// - reconnect interval 

	//
	// connected, connect_failed, accepted, disconnected
	// 
	return true;
}

bool peer_service::init()
{
	// subscribe up / down 

	return true;
}

actor::result peer_service::run() 
{
	// send to peers from queue
	
	return result::success;
}

void peer_service::fini()
{

}

} // kernel 
} // wise
