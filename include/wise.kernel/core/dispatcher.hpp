#pragma once 

#include <wise.kernel/core/detail/sub.hpp>
#include <unordered_map>

namespace wise {
namespace kernel {

/// a simple message dispatcher
/**
 * 간단하게 하나의 클래스에서 사용하는 용도.
 * - 람다 함수로 등록.
 * - 클래스와 수명을 같이 함.
 *
 * 그룹 포스팅은 없음
 */
class dispatcher final
{
public:
	dispatcher() = default;

	~dispatcher()
	{
		clear();
	}

	/// subscribe. unsubscribe는 없음
	bool subscribe(const topic topic, sub::cb_t cb)
	{
		auto iter = subs_.find(topic);

		entry e;

		if (iter != subs_.end())
		{
			e = iter->second;
		}

		e.subs.push_back(
			sub(
				seq_++,	// for debug only
				topic,  // the topic
				[](message::ptr m) { return true;  },  // don't care
				cb,  // callback
				sub::mode::immediate // don't care
			)
		);

		subs_.emplace(topic, e);

		return true;
	}

	/// post.
	std::size_t post(message::ptr m)
	{
		WISE_RETURN_IF(!m->get_topic().is_valid(), 0);

		auto iter = subs_.find(m->get_topic());
		WISE_RETURN_IF(iter == subs_.end(), 0);

		int count = 0;

		auto& subs = iter->second.subs;

		for (auto& sub : subs)
		{
			if (sub.post(m))
			{
				++count;
			}
		}

		return count;
	}

	void clear()
	{
		subs_.clear();
	}

private:
	struct entry
	{
		std::vector<sub>	subs;
	};
	using map = std::unordered_map<topic, entry>;

private:
	map subs_;
	uint32_t seq_ = 1;
};

} // kernel
} // wise
