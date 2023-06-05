#pragma once

#include "Common/Types/Common.h"
#include "Common/Math/Fixed28.h"
#include "Engine/Graphics/Types/Color.h"
#include "Engine/Graphics/Types/Polygon.h"
#include "Engine/Graphics/Scene/Material.h"
#include "Engine/Graphics/Interpolators/IInterpolator.h"
#include "Engine/Graphics/Interpolators/EmissiveInterpolator.h"
#include "Engine/Graphics/Interpolators/FlatInterpolator.h"
#include "Engine/Graphics/Interpolators/GouraudInterpolator.h"
#include "Engine/Graphics/Interpolators/AffineTextureInterpolator.h"
#include "Engine/Graphics/Interpolators/LinearPiecewiseTextureInterpolator.h"
#include "Engine/Graphics/Interpolators/PerspectiveCorrectTextureInterpolator.h"
#include "Engine/Graphics/Interpolators/BilinearPerspectiveTextureInterpolator.h"
#include "Engine/Graphics/Interpolators/AlphaInterpolator.h"

namespace Volition
{

class VInterpolationContext
{
public:
    static constexpr i32f MaxInterpolators = 8;

public:
    u32* Buffer;
    i32 BufferPitch;

    const VVertex* Vtx;
    const VMaterial* Material;

    VColorARGB OriginalColor;
    VColorARGB LitColor[3];

    u32 MaterialAttr;

    f32 Distance;
    i32 MipMappingLevel;

    i32 VtxIndices[3];

    VColorARGB Pixel;
    i32f X, Y;
    fx28 Z;

    IInterpolator* Interpolators[MaxInterpolators];
    i32 NumInterpolators;

    VEmissiveInterpolator EmissiveInterpolator;
    VFlatInterpolator FlatInterpolator;
    VGouraudInterpolator GouraudInterpolator;
    VAffineTextureInterpolator AffineTextureInterpolator;
    VLinearPiecewiseTextureInterpolator LinearPiecewiseTextureInterpolator;
    VPerspectiveCorrectTextureInterpolator PerspectiveCorrectTextureInterpolator;
    VBilinearPerspectiveTextureInterpolator BilinearPerspectiveTextureInterpolator;
    VAlphaInterpolator AlphaInterpolator;
};

}