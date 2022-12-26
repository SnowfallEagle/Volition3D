#pragma once

#define VL_IMPL_SDL 1

// MSVC
#ifdef _MSC_VER
# define VL_PLATFORM_WIN 1
# define VL_COMPILER_MSVC 1
# include "Core/Impl/Platform_Win.h"
#endif

#define VL_LITTLE_ENDIAN 1
#define VL_BIG_ENDIAN 0

// Some macroses
#define INLINE inline // Compiler decide if it should be inlined

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define SWAP(A, B, T) { T = A; A = B; B = T; }

#define BIT(N) ( 1 << ((N)-1) )

#define SAFE_DELETE(X) \
	{ \
		if ((X)) \
		{ \
			delete (X); \
			X = nullptr; \
		} \
	}

#define SAFE_DELETE_ARRAY(X) \
	{ \
		if ((X)) \
		{ \
			delete[] (X); \
			X = nullptr; \
		} \
	}
