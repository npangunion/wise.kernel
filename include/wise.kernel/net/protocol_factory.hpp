#pragma once 

#include <wise.kernel/net/core/protocol.hpp>

#include <functional>
#include <map>

namespace wise {
namespace kernel {

class protocol_factory
{
public:
	using creator = std::function<protocol::ptr()>;

public:
	static protocol_factory& inst();

	/// add a creator for a protocol
	void add(const std::string& name, creator c);

	/// check
	bool has(const std::string& name);

	/// create a protocol
	protocol::ptr create(const std::string& name) const;

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
