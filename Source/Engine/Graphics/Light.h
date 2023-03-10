#pragma once

#include "Engine/Core/Platform.h"
#include "Engine/Core/Types.h"
#include "Engine/Math/Vector.h"
#include "Engine/Graphics/Color.h"

namespace ELightState
{
    enum
    {
        Active = VLN_BIT(1)
    };
}

namespace ELightAttr
{
    enum
    {
        Ambient          = VLN_BIT(1),
        Infinite         = VLN_BIT(2),
        Point            = VLN_BIT(3),
        SimpleSpotlight  = VLN_BIT(4),
        ComplexSpotlight = VLN_BIT(5)
    };
}

class VLight
{
public:
    i32 ID;
    u32 State;
    u32 Attr;

    VColorARGB CAmbient, CDiffuse, CSpecular;
    VPoint4 Pos, TransPos;
    VVector4 Dir, TransDir;

    f32 KConst, KLinear, KQuad;
    f32 SpotInner, SpotOuter;
    f32 Power; // Falloff for spotlights
};