#pragma once

#include "Engine/Core/Platform.h"
#include "Engine/Core/Types.h"
#include "Engine/Math/Vector.h"
#include "Engine/Graphics/Color.h"

namespace ELightState
{
    enum
    {
        Active = VL_BIT(1)
    };
}

namespace ELightAttr
{
    enum
    {
        Ambient          = VL_BIT(1),
        Infinite         = VL_BIT(2),
        Point            = VL_BIT(3),
        SimpleSpotlight  = VL_BIT(4),
        ComplexSpotlight = VL_BIT(5)
    };
}

class VLight
{
public:
    i32 ID;
    u32 State;
    u32 Attr;

    VColorRGBA CAmbient, CDiffuse, CSpecular;
    VPoint4 Pos, TransPos;
    VVector4 Dir, TransDir;

    f32 KConst, KLinear, KQuad;
    f32 SpotInner, SpotOuter;
    f32 Power; // Falloff for spotlights
};