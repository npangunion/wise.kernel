#pragma once 

#include <wise.kernel/server/actor.hpp>
#include <functional>
#include <map>

namespace wise {
namespace kernel {

class actor_factory
{
public:
	using creator = std::function<actor::ptr(server&, actor::id_t id)>;

public:
	static actor_factory& inst();

	/// add a creator for a service
	void add(const std::string& type, creator c);

	/// check
	bool has(const std::string& type);

	/// create a service
	actor::ptr create(
		const std::string& type,
		server& _server,
		actor::id_t id ) const;

	void clear()
	{
		map_.clear();
	}

private:
	using map = std::map<const std::string, creator>;

	map map_;
};

} // kernel
} // wise

#define WISE_ADD_ACTOR(cls) \
  wise::kernel::actor_factory::inst().add(#cls, \
	 [](wise::kernel::server& _server, wise::kernel::actor::id_t id) \
	 { \
		return wise_shared<cls>(_server, id); \
	 })
