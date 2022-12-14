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

class LightV1
{
public:
    u32 State;
    u32 Attr;
    i32 ID;

    VColorRGBA CAmbient, CDiffuse, CSpecular;
    VPoint4D Pos;
};