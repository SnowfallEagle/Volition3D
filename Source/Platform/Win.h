#ifndef PLATFORM_WIN_H_

#include <intrin.h>

#define FINLINE __forceinline

#define DEBUG_BREAK() __debugbreak()
#define PAUSE() _mm_pause()

#define PLATFORM_WIN_H_
#endif