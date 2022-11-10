#pragma once

// Comment/Uncomment to toggle implementation
#define VL_IMPL_SDL 1
//#define VL_IMPL_DDRAW 1

// MSVC
#ifdef _MSC_VER
# define VL_PLATFOM_WIN 1
# define VL_COMPILER_MSVC 1
# include "Core/Impl/Platform_Win.h"
#endif

// Some macroses
#define INLINE inline // Compiler decide if it should be inlined

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define SWAP(A, B, T) { T = A; A = B; B = T; }

#define BIT(N) ( 1 << ((N)-1) )
