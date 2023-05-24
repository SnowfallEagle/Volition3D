#pragma once

#include "Common/Platform/Platform.h"
#include "Common/Types/Common.h"

namespace Volition
{

#define MAP_ARGB32(A, R, G, B) ( (u32)( ((A) << 24) | ((R) << 16) | ((G) << 8) | (B)) )
#define MAP_XRGB32(R, G, B) MAP_ARGB32(0xFF, R, G, B)

class VColorARGB
{
public:
    static VColorARGB Transparent, White, Black, Red, Green, Blue;

public:
    union
    {
        u32 ARGB;
        struct
        {
#if VLN_ENDIANNESS == VLN_LITTLE_ENDIAN
            u8 B, G, R, A;
#else
            u8 A, R, G, B;
#endif
        };
        u8 C[4];
    };

public:
    VLN_FINLINE VColorARGB() = default;
    VLN_FINLINE constexpr VColorARGB(u8 A, u8 R, u8 G, u8 B) noexcept : ARGB(MAP_ARGB32(A, R, G, B)) {}
    VLN_FINLINE constexpr VColorARGB(u32 InARGB) noexcept : ARGB(InARGB) {}

    VLN_FINLINE constexpr operator u32() const noexcept
    {
        return ARGB;
    }

    VLN_FINLINE b32 operator==(const VColorARGB Other) const
    {
        return (u32)*this == (u32)Other;
    }
};

inline VColorARGB VColorARGB::Transparent = MAP_ARGB32(0x00, 0x00, 0x00, 0x00);
inline VColorARGB VColorARGB::White       = MAP_XRGB32(0xFF, 0xFF, 0xFF);
inline VColorARGB VColorARGB::Black       = MAP_XRGB32(0x00, 0x00, 0x00);
inline VColorARGB VColorARGB::Red         = MAP_XRGB32(0xFF, 0x00, 0x00);
inline VColorARGB VColorARGB::Green       = MAP_XRGB32(0x00, 0xFF, 0x00);
inline VColorARGB VColorARGB::Blue        = MAP_XRGB32(0x00, 0x00, 0xFF);

}
