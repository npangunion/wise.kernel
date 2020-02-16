#pragma once 

#include <functional>
#include <vector>

namespace wise
{

/// keep values in vector and call clean functions at destruction
/** 
 * lambda를 사용하여 소멸자에서 뭔가를 처리하는 패턴 중 하나. 
 * lambda의 캡처 기능과 RAII를 구성하는 생성 / 소멸자를 활용
 * 사용성 보다는 패턴 중 하나로 남겨둠
 */
template <typename Value>
class auto_cleaner final
{
public: 
	using cleaner = std::function<void(Value)>;

public: 
	auto_cleaner(cleaner _cleaner)
		: cleaner_(std::move(_cleaner))
	{
	}

	~auto_cleaner()
	{
		for (auto& v : values_)
		{
			cleaner_(v);
		}

		values_.clear();
	}

	auto_cleaner& operator += (Value v)
	{
		values_.push_back(v);
		return *this;
	}

private: 
	cleaner cleaner_;
	std::vector<Value> values_;
};

} // wise