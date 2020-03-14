#pragma once 
#include <lax/net/protocol/bits/bits_message.hpp>

namespace lax
{
namespace net
{

/// 수신한 메세지의 토픽을 보고 메세지를 생성한다.
class bits_message_factory
{
public:
	using creator = std::function<bits_message::ptr()>;

	static bits_message_factory& inst();

	/// add a creator for a protocol
	void add(const packet::topic_t& topic, creator c);

	/// create a protocol
	bits_message::ptr create(const packet::topic_t& topic) const;

	template <typename Msg>
	std::shared_ptr<Msg> create(const packet::topic_t& topic) const
	{
		auto ptr = create(name);

		return std::static_pointer_cast<Msg>(ptr);
	}

private:
	using map = std::map<const packet::topic_t, creator>;

	map map_;
};

} // net
} // lax


#define BITS_MSG_REG(cls) \
lax::net::bits_message_factory::inst().add( \
	cls::topic_t(cls::topic_key),  \
	[]() { return std::make_shared<cls>(); } \
)

#define BITS_MSG_CREATE(topic) \
lax::net::bits_message_factory::inst().create(lax::net::packet::topic_t(topic))