#pragma once

#include "Core/Platform.h"
#include "Core/Types.h"

namespace EMatStateV1
{
    enum
    {
        Active = BIT(1)
    };
}

namespace EMatAttrV1
{
    enum
    {
        RGB32       = BIT(1),
        TwoSided    = BIT(2),
        Transparent = BIT(3),

        ShadeModeConstant  = BIT(4),
        ShadeModeEmissive  = ShadeModeConstant,
        ShadeModeFlat      = BIT(5),
        ShadeModeGouraud   = BIT(6),
        ShadeModeFastPhong = BIT(7),
        ShadeModeTexture   = BIT(8),
    };
}

class VMatV1
{
public:
    static constexpr i32f MaxMaterials = 256;
    static constexpr i32f NameSize = 64;

public:
    i32 ID;
    char Name[NameSize];
    i32 State;
    i32 Attr;
};
