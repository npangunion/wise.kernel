#pragma once 

#include <functional>
#include <vector>

namespace wise
{

/// keep values in vector and call clean functions at destruction
/** 
 * lambda�� ����Ͽ� �Ҹ��ڿ��� ������ ó���ϴ� ���� �� �ϳ�. 
 * lambda�� ĸó ��ɰ� RAII�� �����ϴ� ���� / �Ҹ��ڸ� Ȱ��
 * ��뼺 ���ٴ� ���� �� �ϳ��� ���ܵ�
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