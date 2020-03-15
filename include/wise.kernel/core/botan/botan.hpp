#pragma once

#include <wise.kernel/core/config.hpp>

#if defined(BOTAN_X86)
#include "arch/x86/botan_all.h"
#elif defined(BOTAN_X64)
#include "arch/x64/botan_all.h"
#else // more options
#endif

