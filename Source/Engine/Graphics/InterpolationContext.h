#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Fixed28.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Polygon.h"
#include "Engine/Graphics/Material.h"

class VRenderSpecification;
class IInterpolator;

class VInterpolationContext
{
public:
    const VRenderSpecification& RenderSpec;

    // RenderContext sets this up every frame
    u32* Buffer;
    i32 BufferPitch;

    VVertex* Vtx;
    const VMaterial* Material;

    VColorARGB OriginalColor;
    VColorARGB LitColor[3];

    u32 PolyAttr;

    f32 Distance;
    i32 MipMappingLevel;

    // Renderer sets this up every polygon face
    i32 VtxIndices[3];

    // Renderer sets this up during rasterization
    VColorARGB Pixel;
    i32f X, Y;
    fx28 Z;

public:
    VInterpolationContext(const VRenderSpecification& InRenderSpec) : RenderSpec(InRenderSpec)
    {}
};