#include <pch.hpp>
#include <wise.kernel/server/actor.hpp>
#include <wise.kernel/core/fmt.hpp>

namespace wise
{
namespace kernel 
{

actor::actor(server& _server, id_t parent, id_t id)
	: ch_(fmt::format("actor_{:x}", id))
	, server_(_server)
	, parent_(parent)
	, id_(id)
{
	set_desc(fmt::format("actor_{:x}", id));
}

actor::actor(server& _server, id_t id)
	: ch_(fmt::format("actor_{}", id))
	, server_(_server)
	, parent_(0)
	, id_(id)
{
	set_desc(fmt::format("actor_{:x}", id));
}

actor::~actor()
{
}

bool actor::on_start()
{
	return init();
}

task::result actor::on_execute()
{
	ch_.execute();
	timer_.execute();

	return run();
}

void actor::on_finish()
{
	fini();
}

} // kernel
} // wise
