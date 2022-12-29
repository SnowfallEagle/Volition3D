/* TODO:
	- MemCopy()
	- Assembly version of MemSetByte()?
 */

#pragma once

#include <cstring>
#include "Core/Platform.h"
#include "Core/Types.h"

class VMemory
{
public:
#ifdef VL_COMPILER_MSVC
    FINLINE
#endif
    static void MemSetQuad(void* Dest, i32 Value, SizeType Count)
    {
#ifdef VL_COMPILER_MSVC
        __asm
        {
            mov     eax, Value
            mov     edi, Dest
            mov     ecx, Count
            rep     stosd
        }
#else
        u32* Buffer = (u32*)Dest;
        for ( ; Count; --Count, ++Buffer)
        {
            *Buffer = Value;
        }
#endif
    }

    FINLINE static void MemSetByte(void* Dest, i32 Value, SizeType Count)
    {
        std::memset(Dest, Value, Count);
    }

	FINLINE static void MemCopy(void* Dest, const void* Source, SizeType Size)
	{
		std::memcpy(Dest, Source, Size);
	}
};

extern VMemory Memory;