#pragma once

#include <stack>

namespace wise
{

// �������� �ڵ� ������ �ʿ䰡 �ִ� ��쿡 ���
template <typename Func> 
class cleanup_stack
{
public: 
	cleanup_stack() = default; 

	~cleanup_stack()
	{
		if (failed_)
		{
			while (!stack_.empty())
			{
				Func& func = stack_.top();

				func();

				stack_.pop();
			}
		}
	}

	void push(Func func)
	{
		stack_.push(func);
	}

	bool set_fail()
	{
		failed_ = true;
		return true;
	}

private: 
	std::stack<Func> stack_;
	bool failed_ = false;
};


} // wise