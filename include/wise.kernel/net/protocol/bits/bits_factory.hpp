#pragma once 
#include <wise/net/protocol/zen/zen_message.hpp>

namespace wise
{

/// 수신한 메세지의 토픽을 보고 메세지를 생성한다.
class zen_factory
{
public:
	using creator = std::function<zen_message::ptr()>;

	static zen_factory& inst();

	/// add a creator for a protocol
	void add(const topic& pic, creator c);

	/// create a protocol
	zen_message::ptr create(const topic& pic) const;

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

} // wise


#define ZEN_MSG_CREATE(pic) \
wise::zen_factory::inst().create(wise::topic(pic))

