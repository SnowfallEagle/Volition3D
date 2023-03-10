#pragma once

#include "Engine/Graphics/ZBuffer.h"
#include "Engine/Graphics/InterpolationContext.h"
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

class VRenderList;

class VRenderContext
{
public:
    static constexpr i32f MaxInterpolators = 8;

public:
    const VRenderSpecification& RenderSpec;
    VInterpolationContext InterpolationContext;

    VRenderList* RenderList;
    VZBuffer ZBuffer;

    IInterpolator* Interpolators[MaxInterpolators];
    i32 NumInterpolators;

    VFlatInterpolator FlatInterpolator;
    VGouraudInterpolator GouraudInterpolator;
    VAffineTextureInterpolator AffineTextureInterpolator;
    VLinearPiecewiseTextureInterpolator LinearPiecewiseTextureInterpolator;
    VPerspectiveCorrectTextureInterpolator PerspectiveCorrectTextureInterpolator;
    VBillinearPerspectiveTextureInterpolator BillinearPerspectiveTextureInterpolator;
    VAlphaInterpolator AlphaInterpolator;

public:
    VRenderContext(const VRenderSpecification& InRenderSpec) :
        RenderSpec(InRenderSpec), InterpolationContext(InRenderSpec)
    {}

    void Init();
    void Destroy();

    void PrepareToRender();
    void RenderWorld(u32* Buffer, i32 Pitch);
    void SetInterpolators();

private:
    void RenderSolid();
    void RenderWire();
};

}