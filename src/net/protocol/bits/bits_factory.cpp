#include <pch.hpp>
#include <wise.kernel/net/protocol/bits/bits_factory.hpp>
#include <wise.kernel/core/logger.hpp>
#include <wise.kernel/core/exception.hpp>

namespace wise {
namespace kernel {

bits_factory& bits_factory::inst()
{
	static bits_factory inst_;

	return inst_;
}

void bits_factory::add(const topic& topic, creator c)
{
	auto iter = map_.find(topic);

	if (iter != map_.end())
	{
		WISE_WARN(
			"bits_message w/ topic[{}:{}:{}] is alreay added!",
			topic.get_category(), topic.get_group(), topic.get_type());

		// �׽�Ʈ ���� ���� replace�� �ʿ��� ��찡 ���� 
		// �Ϲ����� ������� ���� ���� ���� (���� �� ������ ��� �� �̽��� ����)
	}

	map_[topic] = c;
}

bits_message::ptr bits_factory::create(const topic& topic) const
{
	auto iter = map_.find(topic);
	WISE_RETURN_IF(iter == map_.end(), bits_message::ptr());

	return iter->second();
}

} // kernel
} // wise 