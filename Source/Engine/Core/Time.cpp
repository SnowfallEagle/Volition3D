#include "Common/Platform/Memory.h"
#include "Engine/Core/Config/Config.h"
#include "Engine/Core/Time.h"

namespace Volition
{

void VTime::StartUp()
{
    MsFrameLimit = 1000 / Config.RenderSpec.TargetFPS;

    LastTick = 0;
    DeltaTime = 0.0f;

    FixedDeltaTime = 1000.0f / Config.RenderSpec.TargetFixedFPS;
    AccumulatedFixedTime = 0.0f;

    Memory.MemSetByte(DeltaTimeCache, 0, sizeof(DeltaTimeCache));
    DeltaTimeCacheIndex = 0;
    FPS = 0.0f;
}

void VTime::TickFrame()
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

void VTime::SyncFrame()
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