#pragma once

#include "SDL.h"
#include "Engine/Core/Platform.h"
#include "Engine/Core/Types.h"
#include "Engine/Graphics/RenderSpecification.h"

namespace Volition
{

class VTime
{
    i32 MsFrameLimit;
    i32 LastTick;
    f32 DeltaTime;

    b32 bLimitFPS;

public:
    void StartUp(const VRenderSpecification& RenderSpec);
    void ShutDown();

    void TickFrame();
    void SyncFrame();

    VLN_FINLINE i32 GetTicks()
    {
        return SDL_GetTicks();
    }

    VLN_FINLINE f32 GetDeltaTime()
    {
        return DeltaTime;
    }
};

extern VTime Time;

}