#pragma once

#include "Engine/Graphics/ZBuffer.h"

class VRenderList;

class VRenderContext
{
public:
    VRenderList* RenderList;
    VZBuffer ZBuffer;

public:
    void Init(i32 ScreenWidth, i32 ScreenHeight);
    void Destroy();

    void PrepareToRender();
    void RenderWorld(u32* Buffer, i32 Pitch);
};