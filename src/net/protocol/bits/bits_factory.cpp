#include "stdafx.h"
#include <wise/net/protocol/zen/zen_factory.hpp>
#include <wise/base/logger.hpp>
#include <wise/base/exception.hpp>

namespace wise
{

zen_factory& zen_factory::inst()
{
	static zen_factory inst_;

	return inst_;
}

void zen_factory::add(const topic& topic, creator c)
{
	auto iter = map_.find(topic);

	if (iter != map_.end())
	{
		WISE_WARN(
			"zen_message w/ topic[{}:{}:{}] is alreay added!",
			topic.get_category(), topic.get_group(), topic.get_type());

		// �׽�Ʈ ���� ���� replace�� �ʿ��� ��찡 ���� 
		// �Ϲ����� ������� ���� ���� ���� (���� �� ������ ��� �� �̽��� ����)
	}

	map_[topic] = c;
}

zen_message::ptr zen_factory::create(const topic& topic) const
{
	auto iter = map_.find(topic);
	WISE_RETURN_IF(iter == map_.end(), zen_message::ptr());

	return iter->second();
}

} // wise 