#pragma once 

namespace wise {
namespace kernel {

/// result and value passing.   
template <typename Code, typename Value>
struct result
{
	Code success = Code();
	Code code = Code();
	Value value = Value();

	result() = default;

	result(Code acode, const Value& avalue)
		: code(acode)
		, value(avalue)
	{
	}

	result(Code acode, const Value&& avalue)
		: code(acode)
		, value(std::move(avalue))
	{
	}

	operator bool() const
	{
		if (std::is_integral<Code>::value)
		{
			return code != 0;
		}

		return code == success;
	}
};

template <typename Value>
struct result<bool, Value>
{
	bool code = true;
	Value value = Value();

	result() = default;

	result(bool acode, const Value& avalue)
		: code(acode)
		, value(avalue)
	{
	}

	result(bool acode, const Value&& avalue)
		: code(acode)
		, value(std::move(avalue))
	{
	}

	operator bool() const
	{
		return code;
	}
};

} // kernel
} // wise
