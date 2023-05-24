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

}