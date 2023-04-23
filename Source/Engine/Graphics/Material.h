#pragma once

#include "Engine/Core/Platform.h"
#include "Engine/Core/Types.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Texture.h"

namespace Volition
{

namespace EMaterialState
{
    enum
    {
        Active = VLN_BIT(1)
    };
}

namespace EMaterialAttr
{
    enum
    {
        RGB32       = VLN_BIT(1),
        TwoSided    = VLN_BIT(2),
        Transparent = VLN_BIT(3),

        ShadeModeEmissive = VLN_BIT(4),
        ShadeModeFlat     = VLN_BIT(5),
        ShadeModeGouraud  = VLN_BIT(6),
        ShadeModePhong    = VLN_BIT(7),
        ShadeModeTexture  = VLN_BIT(8),
    };
}

class VMaterial
{
public:
    static constexpr i32f NameSize = 64;

public:
    i32 ID;
    char Name[NameSize];

    i32 State;
    i32 Attr;

    VColorARGB Color;
    f32 KAmbient, KDiffuse, KSpecular, Power;
    VColorARGB RAmbient, RDiffuse, RSpecular; // K * Color

    VTexture Texture;
};

}
