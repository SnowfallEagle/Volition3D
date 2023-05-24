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

}