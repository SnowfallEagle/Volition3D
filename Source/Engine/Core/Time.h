#pragma once

#include "SDL.h"
#include "Common/Platform/Platform.h"
#include "Common/Types/Common.h"
#include "Engine/Core/Config/Config.h"

namespace Volition
{

class VTime
{
    static constexpr i32f MaxCachedDeltaTimes = 10;

private:
    i32 MsFrameLimit;
    i32 LastTick;
    f32 DeltaTime;

    f32 FixedDeltaTime;
    i32f NumFixedUpdates;
    f32 AccumulatedFixedTime;

    f32 DeltaTimeCache[MaxCachedDeltaTimes];
    i32 DeltaTimeCacheIndex;
    f32 FPS;

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

    VLN_FINLINE f32 GetFPS() const
    {
        return FPS;
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

    DeltaTimeCache[DeltaTimeCacheIndex] = DeltaTime;
    DeltaTimeCacheIndex = (DeltaTimeCacheIndex + 1) % MaxCachedDeltaTimes;

    f32 SumDeltaTimes = 0.0f;
    for (i32f i = 0; i < MaxCachedDeltaTimes; ++i)
    {
        SumDeltaTimes += DeltaTimeCache[i];
    }

    FPS = 1000.0f / (SumDeltaTimes / (f32)MaxCachedDeltaTimes);
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