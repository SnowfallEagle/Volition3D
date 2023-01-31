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
    VL_FINLINE
#endif
    static void MemSetQuad(void* Dest, i32 Value, VSizeType Count)
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

    VL_FINLINE static void MemSetByte(void* Dest, i32 Value, VSizeType Count)
    {
        std::memset(Dest, Value, Count);
    }

	VL_FINLINE static void MemCopy(void* Dest, const void* Source, VSizeType Size)
	{
		std::memcpy(Dest, Source, Size);
	}
};

extern VMemory Memory;