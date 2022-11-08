#ifndef CORE_PLATFORM_H_

// Comment/Uncomment to toggle implementation
#define VL_IMPL_SDL 1
//#define VL_IMPL_DDRAW 1

// MSVC
#ifdef _MSC_VER
# define VL_PLATFOM_WIN 1
# define VL_COMPILER_MSVC 1
# include "Core/Impl/Platform_Win.h"
#endif

// Standard stuff
#define INLINE inline // Compiler decide if it should be inlined

#define CORE_PLATFORM_H_
#endif
