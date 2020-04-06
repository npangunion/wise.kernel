#pragma once

#include <wise/base/result.hpp>
#include <boost/any.hpp>
#include <map>

namespace wise
{

class dic_any
{
public: 
	using key = std::string;

public: 
	template <typename T>
	void set(key k, const T v)
	{
		dic_[k] = boost::any(std::move(v));
	}

	template <typename T>
	result<bool, T> get(key k)
	{
		try
		{
			auto iter = dic_.find(k);
			if (iter == dic_.end())
			{
				return result<bool, T>(false, T());
			}

			auto& av = iter->second;

			return result<bool, T>(true, boost::any_cast<T>(av));
		}
		catch (const boost::bad_any_cast&) 
		{
			return result<bool, T>(false, T());
		}
	}

	void clear(key k)
	{
		dic_.erase(k);
	}

private: 
	using map = std::map<key, boost::any>;

	map dic_;
};

} // wise
