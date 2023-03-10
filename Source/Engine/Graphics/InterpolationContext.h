#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Fixed28.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Polygon.h"
#include "Engine/Graphics/Material.h"

namespace Volition
{

class VRenderSpecification;
class IInterpolator;

class VInterpolationContext
{
public:
    const VRenderSpecification& RenderSpec;

    u32* Buffer;
    i32 BufferPitch;

    VVertex* Vtx;
    const VMaterial* Material;

    VColorARGB OriginalColor;
    VColorARGB LitColor[3];

    u32 PolyAttr;

    f32 Distance;
    i32 MipMappingLevel;

    i32 VtxIndices[3];

    VColorARGB Pixel;
    i32f X, Y;
    fx28 Z;

public:
    VInterpolationContext(const VRenderSpecification& InRenderSpec) : RenderSpec(InRenderSpec)
    {}
};

}