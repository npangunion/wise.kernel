#pragma once 

#include <wise.kernel/base/macros.hpp>
#include <wise.kernel/base/exception.hpp>
#include <wise.kernel/base/null_mutex.hpp>

#include <spdlog/fmt/fmt.h>
#include <algorithm>
#include <deque>
#include <exception>
#include <mutex>


namespace wise {
	namespace kernel {

		/// id pool to reuse ids. 
		/**
		 * 사용 후 release로 재사용 가능하게 해야 함
		 *
		 * Seq - number type
		 * Mutex - Lockable
		 */
		template <typename Seq, typename Mutex = null_mutex>
		class sequence
		{
		public:

			sequence(Seq begin, Seq end, Seq reserve = default_reserve)
				: begin_(begin)
				, end_(end)
				, reserve_(std::min<Seq>(end - begin, reserve))
				, current_(begin)
				, seqs_()
			{
				static_assert(std::is_integral<Seq>::value, "integral required");

				WISE_ASSERT(begin_ < end_);
				WISE_ASSERT(current_ == begin_);

				Seq max_range = end_ - begin_ + 1;
				WISE_ASSERT(reserve_ < max_range);

				acquire(reserve_);

				WISE_ASSERT(!seqs_.empty());
			}

			~sequence()
			{
				seqs_.clear();
			}

			/// 사용 가능한 다음 차례의 숫자 돌려줌. 
			/**
			 *
			 * @return - 다음 차례 숫자. 부족하면 exception
			 */
			Seq next()
			{
				std::lock_guard<Mutex> lock(mutex_);

				if (seqs_.size() < reserve_)
				{
					acquire(reserve_);
				}

				Seq value = Seq();

				if (!seqs_.empty())
				{
					value = seqs_.front();

					seqs_.pop_front();
				}
				else
				{
					if (current_ == end_)
					{
						WISE_THROW(
							fmt::format("no sequence to use. begin:{}, end:{}, current:{}",
								begin_, end_, current_).c_str()
						);
					}

					value = current_++;
				}

				return value;
			}

			void release(Seq seq)
			{
				WISE_ASSERT(begin_ <= seq && seq < end_);
				WISE_RETURN_IF(!(begin_ <= seq && seq < end_));

				std::lock_guard<Mutex> lock(mutex_);
				seqs_.push_back(seq);
			}

			/// 테스트 용으로만 사용
			Seq current() const
			{
				std::lock_guard<Mutex> lock(mutex_);
				return current_;
			}

			std::size_t queue_size() const
			{
				std::lock_guard<Mutex> lock(mutex_);
				return seqs_.size();
			}

		private:
			void acquire(std::size_t count)
			{
				for (std::size_t s = 0; s < count && current_ < end_; ++s)
				{
					seqs_.push_back(current_++);
				}
			}

		private:
			static constexpr Seq default_reserve = 100;

			mutable Mutex	mutex_;
			const Seq		begin_;
			const Seq		end_;
			const Seq		reserve_;
			Seq				current_;
			std::deque<Seq> seqs_;
		};

	} // kernel
} // wise
