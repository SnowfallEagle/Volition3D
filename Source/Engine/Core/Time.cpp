#include "Engine/Core/Config/Config.h"
#include "Engine/Core/Time.h"

namespace Volition
{

VTime Time;

void VTime::StartUp()
{
    MsFrameLimit = 1000 / Config.RenderSpec.TargetFPS;
    bLimitFPS = Config.RenderSpec.bLimitFPS;

    LastTick = 0;
    DeltaTime = 0.0f;

    FixedDeltaTime = 1000.0f / Config.RenderSpec.TargetFixedFPS;
    AccumulatedFixedTime = 0.0f;
}

void VTime::ShutDown()
{
}

}