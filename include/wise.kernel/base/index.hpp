#pragma once

#include <wise.kernel/base/macros.hpp>
#include <unordered_map>

namespace wise {
	namespace kernel {

		//
		// @brief 주키에 대한 보조키를 정의하고 사용한다. 
		//
		template <typename IndexKey, typename PrimaryKey>
		struct index
		{
			using container = std::unordered_map<IndexKey, PrimaryKey>;

			void set(const IndexKey& ik, const PrimaryKey& pk)
			{
				WISE_ASSERT(!has(ik));

				map_[ik] = pk;
			}

			void unset(const IndexKey& ik)
			{
				map_.erase(ik);
			}

			bool has(const IndexKey& ik) const
			{
				auto iter = map_.find(ik);
				return iter != map_.end();
			}

			std::size_t size() const
			{
				return map_.size();
			}

			const PrimaryKey& get(const IndexKey& ik) const
			{
				auto iter = map_.find(ik);
				WISE_RETURN_IF(iter == map_.end(), null_);

				return iter->second;
			}

			void clear()
			{
				map_.clear();
			}

			container map_;
			PrimaryKey null_;
		};

	}
} // wise::kernel