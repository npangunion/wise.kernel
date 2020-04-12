#include <pch.hpp>
#include <wise.kernel/server/actor_factory.hpp>

namespace wise {
namespace kernel {

actor_factory& actor_factory::inst()
{
	static actor_factory inst_;

	return inst_;
}

void actor_factory::add(const std::string& type, creator c)
{
	auto iter = map_.find(type);
	WISE_RETURN_IF(iter != map_.end());

	map_[type] = c;
}

bool actor_factory::has(const std::string& type)
{
	auto iter = map_.find(type);

	return iter != map_.end();
}

actor::ptr actor_factory::create(
	const std::string& type,
	server& _server,
	actor::id_t id) const
{
	auto iter = map_.find(type);
	WISE_RETURN_IF(iter == map_.end(), actor::ptr());

	return iter->second(_server, id);
}

} // kernel
} // wise
