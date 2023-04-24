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

    f32 FixedDeltaTime;
    i32f NumFixedUpdates;
    f32 AccumulatedFixedTime;

    b32 bLimitFPS;

public:
    void StartUp(const VRenderSpecification& RenderSpec);
    void ShutDown();

    VLN_FINLINE void TickFrame()
    {
        const u32 CurrentTick = GetTicks();
        DeltaTime = (f32)(CurrentTick - LastTick);
        LastTick = CurrentTick;

        AccumulatedFixedTime += DeltaTime;
        NumFixedUpdates = (i32f)(AccumulatedFixedTime / FixedDeltaTime);
        AccumulatedFixedTime -= NumFixedUpdates * FixedDeltaTime;
    }

    VLN_FINLINE void SyncFrame()
    {
        if (bLimitFPS)
        {
            while ((i32)GetTicks() - LastTick < MsFrameLimit)
            {
                VLN_PAUSE();
            }
        }
    }

    VLN_FINLINE i32 GetTicks() const
    {
        return SDL_GetTicks();
    }

    VLN_FINLINE f32 GetDeltaTime() const
    {
        return DeltaTime;
    }

    VLN_FINLINE f32 GetFixedDeltaTime() const
    {
        return FixedDeltaTime;
    }

    VLN_FINLINE i32f GetNumFixedUpdates() const
    {
        return NumFixedUpdates;
    }
};

extern VTime Time;

}