#include <pch.hpp>
#include <wise.kernel/base/timer.hpp>
#include <wise.kernel/base/logger.hpp>
#include <algorithm>

namespace
{
std::vector<wise::kernel::tick_t> intervals{
	10,
	20,
	30,
	40,
	50,
	100,
	200,
	300,
	400,
	500,
	1000,
	2000,
	3000,
	5000,
	10 * 1000,
	20 * 1000,
	30 * 1000,
	40 * 1000,
	50 * 1000,
	60 * 1000,			// 1m
	2 * 60 * 1000,		// 2m
	3 * 60 * 1000,		// 3m
	4 * 60 * 1000,		// 4m
	5 * 60 * 1000,		// 5m
	10 * 60 * 1000,		// 10m
	30 * 60 * 1000,		// 30m
	60 * 60 * 1000		// 1h
};
}

namespace wise {
	namespace kernel {

		timer::timer()
			: seq_(1, UINT32_MAX, 16)
		{
		}

		timer::~timer()
		{
			slots_.clear();
			reqs_.clear();

			// 명시적으로 shared_ptr를 정리해 주면 
			// 메모리 추적에 유용하다. 
		}

		int timer::set(tick_t interval, tick_t duration, bool repeat, tick_t after)
		{
			is_set_during_posting_ = true;

			if (interval < min_interval)
			{
				WISE_INFO("changing interval to min_interval {} from {}", min_interval, interval);

				interval = min_interval;
			}

			auto target_interval = interval / 2; // 절반이 되어야 타이머 처리 분산이 됨

			WISE_ASSERT(intervals.size() > 5);

			tick_t fi = intervals[0];

			for (int i = 0; i < intervals.size() - 1; ++i)
			{
				if (intervals[i] <= target_interval && target_interval < intervals[i + 1])
				{
					fi = intervals[i];
					break;
				}
			}

			auto& slt = create_slot(fi);

			auto rp = std::make_shared<req>(*this, seq_.next(), interval, duration, after, repeat);

			reqs_.emplace(rp->id_, rp);

			slt.reqs_.push(slot::entry(reqs_[rp->id_]));

			return rp->id_;
		}

		int timer::once(tick_t after, action act)
		{
			auto id = set(after, 0, false);

			(void)add(id, std::move(act));

			return id;
		}

		int timer::repeat(tick_t interval, action act)
		{
			auto id = set(interval);

			if (add(id, std::move(act)))
			{
				return id;
			}

			return 0;
		}

		bool timer::has(id_t id) const
		{
			return reqs_.find(id) != reqs_.end();
		}

		bool timer::add(id_t id, action&& act)
		{
			auto iter = reqs_.find(id);

			WISE_RETURN_IF(iter == reqs_.end(), false);

			iter->second->add(std::move(act));

			return true;
		}

		bool timer::cancel(id_t id)
		{
			auto iter = reqs_.find(id);

			WISE_RETURN_IF(iter == reqs_.end(), false);

			iter->second->cancel_ = true;	// 취소된 요청들은 slot 실행할 때 제거

			// safe to erase 
			reqs_.erase(iter);

			return true;
		}

		void timer::execute()
		{
			auto tick = get_current_tick();

			is_set_during_posting_ = false;

			// 슬롯은 지우지 않기에 인덱스로 접근하면 안전

			auto size = slots_.size();

			for (std::size_t i = 0; i < size; ++i)
			{
				auto& sl = slots_[i];

				update_slot(sl, tick);
			}

			// 빈 슬롯을 지우지 않는다.
		}

		void timer::update_slot(slot& sl, tick_t now)
		{
			if ((sl.last_run_tick_ + sl.interval_) > now)
			{
				return;
			}

			sl.last_run_tick_ = now;

			WISE_RETURN_IF(sl.reqs_.empty());

			auto e = sl.reqs_.top();

			while (e.ptr_->next_run_tick_ < now)
			{
				sl.reqs_.pop();

				if (!e.ptr_->cancel_)
				{
					e.ptr_->last_run_tick_ = now;

					e.ptr_->run();

					if (e.ptr_->repeat_ || now < e.ptr_->end_run_tick_)
					{
						e.ptr_->next_run_tick_ = now + e.ptr_->interval_;

						sl.reqs_.push(e);
					}
					else
					{
						remove_end_req(e.ptr_->id_);
					}
				}

				if (sl.reqs_.empty())
				{
					break;
				}

				e = sl.reqs_.top();
			}
		}


		tick_t timer::get_next_run_tick(id_t id) const
		{
			auto iter = reqs_.find(id);

			WISE_RETURN_IF(iter == reqs_.end(), 0);

			return iter->second->next_run_tick_;
		}

		tick_t timer::get_last_run_tick(id_t id) const
		{
			auto iter = reqs_.find(id);

			WISE_RETURN_IF(iter == reqs_.end(), 0);

			return iter->second->last_run_tick_;
		}

		std::size_t timer::get_run_count(id_t id) const
		{
			auto iter = reqs_.find(id);

			WISE_RETURN_IF(iter == reqs_.end(), 0);

			return iter->second->run_count_;
		}

		timer::slot& timer::create_slot(tick_t interval)
		{
			// interval 보다 작은 간격의 슬롯들 중 최대 슬롯을 찾는다. 
			// 슬롯은 정렬되어 있지 않다.

			slot* found = nullptr;
			tick_t found_interval = 0;

			for (auto& slt : slots_)
			{
				if (slt.interval_ == interval)
				{
					found = &slt;
					found_interval = slt.interval_;

					break;
				}
			}

			// 없거나 해당 슬록의 간격이 interval의 절반 보다 크면 추가 생성한다. 
			if (found == nullptr || found->interval_ > interval)
			{
				slot slt;

				slt.interval_ = interval;
				slt.last_run_tick_ = 0;

				slots_.push_back(slt);

				return *slots_.rbegin();
			}

			WISE_ASSERT(found);

			return *found;
		}

		void timer::remove_end_req(id_t id)
		{
			auto iter = reqs_.find(id);

			WISE_RETURN_IF(iter == reqs_.end());

			reqs_.erase(iter);
		}

		timer::req::req(timer& t,
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

		void timer::req::run()
		{
			WISE_ASSERT(id_ > 0);

			for (auto& act : actions_)
			{
				act(id_);
			}

			++run_count_;
		}

		void timer::req::release()
		{
			if (id_ > 0)
			{
				timer_.seq_.release(id_);
			}

			id_ = 0;
		}

		timer_holder::timer_holder(timer& _timer)
			: timer_(_timer)
		{
		}

		timer_holder::~timer_holder()
		{
			if (id_ > 0)
			{
				timer_.cancel(id_);
			}
		}

		timer::id_t timer_holder::once(tick_t after, timer::action act)
		{
			WISE_THROW_IF(id_ > 0, "timer holder can have only one timer");

			once_ = true;
			id_ = timer_.once(after, WISE_TIMER_CB(on_once));
			once_act_ = std::move(act);

			return id_;
		}

		timer::id_t timer_holder::repeat(tick_t interval, timer::action act)
		{
			id_ = timer_.repeat(interval, act);
			return id_;
		}

		void timer_holder::cancel()
		{
			if (id_ > 0)
			{
				timer_.cancel(id_);
			}

			id_ = 0;
		}

		void timer_holder::on_once(int timer)
		{
			WISE_ASSERT(once_);
			WISE_ASSERT(once_act_);

			if (once_act_)
			{
				once_act_(timer);
			}
		}

	} // kernel
} // wise
