#ifndef CORE_PLATFORM_H_

// Comment/Uncomment to toggle
#define VL_IMPL_SDL 1
//#define VL_IMPL_DDRAW 1

#ifdef _MSC_VER
# define VL_PLATFOM_WIN 1
# define VL_COMPILER_MSVC 1
# include "Platform/Win.h"
#endif

#define CORE_PLATFORM_H_
#endif
