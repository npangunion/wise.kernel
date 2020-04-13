#pragma once

#include <wise.kernel/server/actor.hpp>

namespace wise {
namespace kernel {

class client_service : public actor
{
public:
	client_service(server& _server, id_t id)
		: actor(_server, id)
	{
	}

	bool setup(const nlohmann::json& _json) override;
	
private:
	bool init() override;

	result run() override;

	void fini() override;

private:
	// clients
};

} // kernel
} // wise