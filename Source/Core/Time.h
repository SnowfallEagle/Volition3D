#pragma once

#include "SDL.h"
#include "Core/Platform.h"
#include "Core/Types.h"
#include "Graphics/RenderSpecification.h"

class VTime
{
    i32 MsFrameLimit;
    i32 LastTick;
    f32 DeltaTime;

    b32 bLimitFPS;

public:
    void StartUp(const VRenderSpecification& RenderSpec)
    {
        // TODO(sean): In case where we want to update render specification, we need get info from render context

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
                PAUSE();
            }
        }
    }

    FINLINE i32 GetTicks()
    {
        return SDL_GetTicks();
    }

    FINLINE f32 GetDeltaTime()
    {
        return DeltaTime;
    }
};

extern VTime Time;