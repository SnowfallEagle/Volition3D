#pragma once

#include "Core/Platform.h"
#include "Core/Types.h"
#include "Graphics/Color.h"
#include "Graphics/Surface.h"

namespace EMaterialStateV1
{
    enum
    {
        Active = BIT(1)
    };
}

namespace EMaterialAttrV1
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

    VColorRGBA Color;
    f32 KAmbient, KDiffuse, KSpecular, Power;
    f32 RAmbient, RDiffuse, RSpecular; // K * Color

    char Path[PathSize];
    VSurface Texture;

public:
    VMaterial() = default;
};
