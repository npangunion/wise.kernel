#include <pch.hpp>
#include <wise.kernel/server/client_service.hpp>

namespace wise {
namespace kernel {

bool client_service::setup(const nlohmann::json& _json)
{
	// listen address

	return true;
}

bool client_service::init()
{
	// subscribe accepted / disconnected 

	return true;
}

actor::result client_service::run() 
{
	// heartbeat
	
	return result::success;
}

void client_service::fini()
{

}

} // kernel 
} // wise
