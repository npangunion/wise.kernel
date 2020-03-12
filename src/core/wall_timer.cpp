#include <pch.hpp>
#include <wise.kernel/core/wall_timer.hpp>

namespace wise
{

wall_timer::wall_timer()
	: seq_(1, UINT32_MAX, 16)
{
}

int wall_timer::set(tick_t interval, tick_t duration, bool repeat, tick_t after)
{
	auto si = min_interval;
	auto fi = si;
	auto target_interval = interval / 2; // 절반이 되어야 타이머 처리 분산이 됨

	while (si < target_interval)
	{
		fi = si;
		si *= 2;
	}

	WISE_ASSERT(fi <= interval);
	WISE_ASSERT(fi <= target_interval);

	auto& slt = create_slot(fi);

	auto rp = std::make_shared<req>(*this, seq_.next(), interval, duration, after, repeat);

	reqs_.emplace(rp->id_, rp);

	slt.reqs_.push(slot::entry(reqs_[rp->id_]));

	return rp->id_;
}

bool wall_timer::has(id_t id) const
{
	return reqs_.find(id) != reqs_.end();
}

bool wall_timer::add(id_t id, action act)
{
	auto iter = reqs_.find(id);

	WISE_RETURN_IF(iter == reqs_.end(), false);

	iter->second->add(act);

	return true;
}

bool wall_timer::cancel(id_t id)
{
	auto iter = reqs_.find(id);

	WISE_RETURN_IF(iter == reqs_.end(), false);

	iter->second->cancel_ = true;	// 취소된 요청들은 slot 실행할 때 제거

	// safe to erase 
	reqs_.erase(iter);

	return true;
}

wall_timer::tick_t wall_timer::get_next_run_tick(id_t id) const
{
	auto iter = reqs_.find(id);

	WISE_RETURN_IF(iter == reqs_.end(), 0.f);

	return iter->second->next_run_tick_;
}

wall_timer::tick_t wall_timer::get_last_run_tick(id_t id) const
{
	auto iter = reqs_.find(id);

	WISE_RETURN_IF(iter == reqs_.end(), 0.f);

	return iter->second->last_run_tick_;
}

std::size_t wall_timer::get_run_count(id_t id) const
{
	auto iter = reqs_.find(id);

	WISE_RETURN_IF(iter == reqs_.end(), 0);

	return iter->second->run_count_;
}

wall_timer::slot& wall_timer::create_slot(tick_t interval)
{
	WISE_ASSERT(interval >= min_interval);

	// interval 보다 작은 간격의 슬롯들 중 최대 슬롯을 찾는다. 
	// 슬롯은 정렬되어 있지 않다.

	slot* found = nullptr;
	tick_t found_interval = 0;

	for (auto& slt : slots_)
	{
		if (slt.interval_ < interval)
		{
			if (found_interval < slt.interval_)
			{
				found = &slt;
				found_interval = slt.interval_;
			}
		}
	}

	// 없거나 해당 슬록의 간격이 interval의 절반 보다 크면 추가 생성한다. 
	if (found == nullptr || found->interval_ > interval )
	{
		slot slt;
		
		slt.interval_ = interval;
		slt.last_run_tick_ = 0.f;

		slots_.push_back(slt);

		return *slots_.rbegin();
	}

	WISE_ASSERT(found);

	return *found;
}

void wall_timer::remove_end_req(id_t id)
{
	auto iter = reqs_.find(id);

	WISE_RETURN_IF(iter == reqs_.end());

	reqs_.erase(iter);
}

wall_timer::req::req(wall_timer& t,
	id_t id, 
	tick_t interval, tick_t duration, tick_t after, bool repeat)
	: timer_(t)
	, id_(id)
	, interval_(interval)
	, duration_(duration)
	, after_(after)
	, repeat_(repeat)
{
	WISE_ASSERT(id_ > 0);
	WISE_ASSERT(interval_ > 0);
	WISE_ASSERT(duration_ >= 0);
	WISE_ASSERT(after_ >= 0);

	interval_ = std::max(min_interval, interval_);			// clamp

	next_run_tick_ = t.get_current_tick() + after_ + interval_;
	end_run_tick_ = t.get_current_tick() + after_ + duration_;
}

void wall_timer::req::run()
{
	WISE_ASSERT(id_ > 0);

	for (auto& act : actions_)
	{
		act(id_);
	}

	++run_count_;
}

void wall_timer::req::release()
{
	if (id_ > 0)
	{
		timer_.seq_.release(id_);
	}

	id_ = 0;
}


} // wise
