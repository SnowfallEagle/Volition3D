#pragma once

#include "Engine/Graphics/ZBuffer.h"
#include "Engine/Graphics/InterpolationContext.h"

class VRenderList;

class VRenderContext
{
public:
    VRenderList* RenderList;
    VZBuffer ZBuffer;

    const VRenderSpecification& RenderSpec;
    VInterpolationContext InterpolationContext;

public:
    VRenderContext(const VRenderSpecification& InRenderSpec) :
        RenderSpec(InRenderSpec), InterpolationContext(InRenderSpec)
    {}

    void Init();
    void Destroy();

    void PrepareToRender();
    void RenderWorld(u32* Buffer, i32 Pitch);

private:
    void RenderSolid();
    void RenderWire();
};