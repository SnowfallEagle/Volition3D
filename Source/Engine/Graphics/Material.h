#pragma once

#include "Engine/Core/Platform.h"
#include "Engine/Core/Types.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Texture.h"

namespace EMaterialState
{
    enum
    {
        Active = VL_BIT(1)
    };
}

namespace EMaterialAttr
{
    enum
    {
        RGB32       = VL_BIT(1),
        TwoSided    = VL_BIT(2),
        Transparent = VL_BIT(3),

        ShadeModeEmissive = VL_BIT(4),
        ShadeModeFlat     = VL_BIT(5),
        ShadeModeGouraud  = VL_BIT(6),
        ShadeModePhong    = VL_BIT(7),
        ShadeModeTexture  = VL_BIT(8),
    };
}

class VMaterial
{
public:
    static constexpr i32f NameSize = 64;
    static constexpr i32f PathSize = 80;

public:
    i32 ID;
    char Name[NameSize];

    i32 State;
    i32 Attr;

    VColorARGB Color;
    f32 KAmbient, KDiffuse, KSpecular, Power;
    VColorARGB RAmbient, RDiffuse, RSpecular; // K * Color

    VTexture Texture;

public:
    VMaterial() = default;
};
