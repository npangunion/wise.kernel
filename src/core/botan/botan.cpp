#include <pch.hpp>
#include <wise.kernel/core/botan/botan.hpp>

#ifdef _MSC_VER
#	pragma warning(disable: 4127) // 조건식이 상수입니다. 
#endif

#if defined(BOTAN_X86)
#include "arch/x86/botan_all.cpp"
#elif defined(BOTAN_X64)
#include "arch/x64/botan_all.cpp"
#else // more options
#endif



