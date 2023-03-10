#pragma once

#include <cstring>
#include "Engine/Core/Platform.h"
#include "Engine/Core/Types.h"

namespace Volition
{

class VMemory
{
public:
#ifdef VLN_COMPILER_MSVC
    VLN_FINLINE
#endif
    static void MemSetQuad(void* Dest, i32 Value, VSizeType Count)
    {
#ifdef VLN_COMPILER_MSVC
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

    VLN_FINLINE static void MemSetByte(void* Dest, i32 Value, VSizeType Count)
    {
        std::memset(Dest, Value, Count);
    }

	VLN_FINLINE static void MemCopy(void* Dest, const void* Source, VSizeType Size)
	{
		std::memcpy(Dest, Source, Size);
	}
};

extern VMemory Memory;

}