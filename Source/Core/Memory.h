#pragma once

#include "Core/Platform.h"
#include "Core/Types.h"

class VMemory
{
public:
#ifdef VL_COMPILER_MSVC
    FINLINE
#endif
    static void MemSetQuad(void* Dest, i32 Value, i32 Count)
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
};

extern VMemory Memory;