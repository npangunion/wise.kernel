#pragma once 

#include <wise.kernel/net/protocol/bits/bits_packet.hpp>

namespace wise {
namespace kernel {

/// 수신한 메세지의 토픽을 보고 메세지를 생성한다.
class bits_factory
{
public:
	using creator = std::function<bits_packet::ptr()>;

	static bits_factory& inst();

	/// add a creator for a protocol
	void add(const topic& pic, creator c);

	/// create a protocol
	bits_packet::ptr create(const topic& pic) const;

	template <typename Msg>
	std::shared_ptr<Msg> create(const topic& pic) const
	{
		auto ptr = create(pic);

		return std::static_pointer_cast<Msg>(ptr);
	}

private:
	using map = std::map<const topic, creator>;

	map map_;
};

} // kernel
} // wise


#define BITS_MSG_CREATE(pic) \
wise::kernel::bits_factory::inst().create(wise::kernel::topic(pic))

