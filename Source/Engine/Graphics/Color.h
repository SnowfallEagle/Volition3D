#pragma once

#include "Engine/Core/Platform.h"
#include "Engine/Core/Types.h"

#define MAP_ARGB32(A, R, G, B) ( (u32)( ((A) << 24) | ((R) << 16) | ((G) << 8) | (B)) )
#define MAP_XRGB32(R, G, B) MAP_ARGB32(0xFF, R, G, B)

class VColorARGB
{
public:
    union
    {
        u32 ARGB;
        struct
        {
#if VL_ENDIANNESS == VL_LITTLE_ENDIAN
            u8 B, G, R, A;
#else
            u8 A, R, G, B;
#endif
        };
        u8 C[4];
    };

public:
    VL_FINLINE VColorARGB() = default;
    VL_FINLINE VColorARGB(u8 A, u8 R, u8 G, u8 B) :
        ARGB(MAP_ARGB32(A, R, G, B))
    {}
    VL_FINLINE VColorARGB(u32 InARGB) :
        ARGB(InARGB)
    {}

    VL_FINLINE operator u32() const
    {
        return ARGB;
    }
};
