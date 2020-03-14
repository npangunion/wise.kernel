#include <pch.hpp>
#include <wise.kernel/net/protocol_factory.hpp>

namespace wise {
namespace kernel {

protocol_factory& protocol_factory::inst()
{
	static protocol_factory inst_;

	return inst_;
}

void protocol_factory::add(const std::string& name, creator c)
{
	auto iter = map_.find(name);
	WISE_RETURN_IF(iter != map_.end());

	map_[name] = c;
}

bool protocol_factory::has(const std::string& name)
{
	auto iter = map_.find(name);

	return iter != map_.end();
}

protocol::ptr protocol_factory::create(const std::string& name) const
{
	auto iter = map_.find(name);
	WISE_RETURN_IF(iter == map_.end(), protocol::ptr());

	return iter->second();
}

} // kernel
} // wise
