#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Fixed28.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Polygon.h"
#include "Engine/Graphics/Material.h"
#include "Engine/Graphics/Interpolators/IInterpolator.h"
#include "Engine/Graphics/Interpolators/FlatInterpolator.h"
#include "Engine/Graphics/Interpolators/GouraudInterpolator.h"
#include "Engine/Graphics/Interpolators/AffineTextureInterpolator.h"
#include "Engine/Graphics/Interpolators/LinearPiecewiseTextureInterpolator.h"
#include "Engine/Graphics/Interpolators/PerspectiveCorrectTextureInterpolator.h"
#include "Engine/Graphics/Interpolators/BillinearPerspectiveTextureInterpolator.h"
#include "Engine/Graphics/Interpolators/AlphaInterpolator.h"

namespace Volition
{

class VRenderSpecification;
class IInterpolator;

class VInterpolationContext
{
public:
    static constexpr i32f MaxInterpolators = 8;

public:
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

    IInterpolator* Interpolators[MaxInterpolators];
    i32 NumInterpolators;

    VFlatInterpolator FlatInterpolator;
    VGouraudInterpolator GouraudInterpolator;
    VAffineTextureInterpolator AffineTextureInterpolator;
    VLinearPiecewiseTextureInterpolator LinearPiecewiseTextureInterpolator;
    VPerspectiveCorrectTextureInterpolator PerspectiveCorrectTextureInterpolator;
    VBillinearPerspectiveTextureInterpolator BillinearPerspectiveTextureInterpolator;
    VAlphaInterpolator AlphaInterpolator;
};

}