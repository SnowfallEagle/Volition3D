#include "Engine/Core/Time.h"

namespace Volition
{

VTime Time;

void VTime::StartUp(const VRenderSpecification& RenderSpec)
{
    MsFrameLimit = 1000 / RenderSpec.TargetFPS;
    bLimitFPS = RenderSpec.bLimitFPS;

    LastTick = 0;
    DeltaTime = 0.0f;

    FixedDeltaTime = 1000.0f / RenderSpec.TargetFixedFPS;
    AccumulatedFixedTime = 0.0f;
}

void VTime::ShutDown()
{
}

}