#pragma once

#include "Core/Platform.h"
#include "Core/Types.h"
#include "Math/Vector.h"
#include "Graphics/Color.h"

namespace ELightStateV1
{
    enum
    {
        Active = BIT(1)
    };
}

namespace ELightAttrV1
{
    enum
    {
        Ambient          = BIT(1),
        Infinite         = BIT(2),
        Point            = BIT(3),
        SimpleSpotlight  = BIT(4),
        ComplexSpotlight = BIT(5)
    };
}

class VLightV1
{
public:
    i32 ID;
    u32 State;
    u32 Attr;

    VColorRGBA CAmbient, CDiffuse, CSpecular;
    VPoint4D Pos;
    VVector4D Dir;

    f32 KConst, KLinear, KQuad;
    f32 SpotInner, SpotOuter;
    f32 Power; // Falloff for spotlights
};