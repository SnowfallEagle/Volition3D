#pragma once

#include "SDL.h"
#include "Common/Platform/Platform.h"
#include "Common/Types/Common.h"
#include "Engine/Core/Config/Config.h"

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

public:
    void StartUp();
    void ShutDown() {}

    void TickFrame();
    void SyncFrame();

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

inline VTime Time;

VLN_FINLINE void VTime::TickFrame()
{
    const u32 CurrentTick = GetTicks();
    DeltaTime = (f32)(CurrentTick - LastTick);
    LastTick = CurrentTick;

    AccumulatedFixedTime += DeltaTime;
    NumFixedUpdates = (i32f)(AccumulatedFixedTime / FixedDeltaTime);
    AccumulatedFixedTime -= NumFixedUpdates * FixedDeltaTime;
}

VLN_FINLINE void VTime::SyncFrame()
{
    if (Config.RenderSpec.bLimitFPS)
    {
        while ((i32)GetTicks() - LastTick < MsFrameLimit)
        {
            VLN_PAUSE();
        }
    }
}

}