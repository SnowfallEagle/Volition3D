#pragma once

#include "Engine/Graphics/ZBuffer.h"

class VRenderList;

class VRenderContext
{
public:
    VRenderList* RenderList;
    VZBuffer ZBuffer;

public:
    void Init(const VRenderSpecification& RenderSpec);
    void Destroy();

    void PrepareToRender();
    void RenderWorld(u32* Buffer, i32 Pitch);
};