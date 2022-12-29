#pragma once

#define VL_IMPL_SDL 1

// MSVC
#ifdef _MSC_VER
# define VL_PLATFORM_WIN 1
# define VL_COMPILER_MSVC 1
# include "Core/Impl/Platform_Win.h"
#endif

// Rasterizer mode
#define VL_RASTERIZER_MODE_ACCURATE 0
#define VL_RASTERIZER_MODE_FAST     1
#define VL_RASTERIZER_MODE_FASTEST  2

#define VL_RASTERIZER_MODE VL_RASTERIZER_MODE_ACCURATE

// Endianness
#define VL_LITTLE_ENDIAN 1
#define VL_BIG_ENDIAN 0

#define VL_ENDIANNESS VL_LITTLE_ENDIAN

// Defines
#define INLINE inline // Compiler decide if it should be inlined

// Macroses
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
