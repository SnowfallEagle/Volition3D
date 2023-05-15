#pragma once

#include "Common/Platform/Platform.h"
#include "Common/Types/Common.h"
#include "Common/Math/Vector.h"
#include "Engine/Graphics/Types/Color.h"

namespace Volition
{

enum class ELightType : u8
{
    Ambient = 0,
    Infinite,
    Point,
    SimpleSpotlight,
    ComplexSpotlight
};

/* @NOTE: What types of lights use
    Ambient:
        Color

    Infinite:
        Color
        Direction

    Point:
        Color
        Position
        KConst, KLinear, KQuad

    SimpleSpotlight:
        Color
        Position
        Direction
        KConst, KLinear, KQuad

    ComplexSpotlight:
        Color
        Position
        Direction
        KConst, KLinear, KQuad
        FalloffPower
*/

class VLight
{
public:
    b8 bActive;
    ELightType Type;

    VColorARGB Color;
    VPoint4 Position, TransPosition;
    VVector4 Direction, TransDirection;

    f32 KConst, KLinear, KQuad;
    f32 FalloffPower;

public:
    void Init(ELightType InType);
};

}