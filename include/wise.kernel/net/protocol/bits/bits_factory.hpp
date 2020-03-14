#pragma once 

#include <wise.kernel/net/protocol/bits/bits_message.hpp>

namespace wise {
namespace kernel {

/// ������ �޼����� ������ ���� �޼����� �����Ѵ�.
class bits_factory
{
public:
	using creator = std::function<bits_message::ptr()>;

	static bits_factory& inst();

	/// add a creator for a protocol
	void add(const topic& pic, creator c);

	/// create a protocol
	bits_message::ptr create(const topic& pic) const;

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

] // kernel
} // wise


#define BITS_MSG_CREATE(pic) \
wise::bits_factory::inst().create(wise::topic(pic))

