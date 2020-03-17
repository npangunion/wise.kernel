#pragma once 

#include <math.h>

namespace wise {
namespace kernel {

static constexpr float epsilon = 0.00001f;

template <typename T>
bool is_equal(T a, T b)
{
	return std::abs(a - b) < epsilon;
}

} // kernel
} // wise