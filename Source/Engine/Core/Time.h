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
    void StartUp(const VRenderSpecification& RenderSpec)
    {
        MsFrameLimit = 1000 / RenderSpec.TargetFPS;
        bLimitFPS = RenderSpec.bLimitFPS;

        LastTick = 0;
        DeltaTime = 0.0f;
    }

    void ShutDown()
    {}

    void TickFrame()
    {
        u32 CurrentTick = GetTicks();
        DeltaTime = (f32)(CurrentTick - LastTick);
        LastTick = CurrentTick;
    }

    void SyncFrame()
    {
        if (bLimitFPS)
        {
            while ((i32)GetTicks() - LastTick < MsFrameLimit)
            {
                VLN_PAUSE();
            }
        }
    }

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