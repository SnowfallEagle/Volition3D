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
}

void VTime::ShutDown()
{
}

void VTime::TickFrame()
{
    const u32 CurrentTick = GetTicks();
    DeltaTime = (f32)(CurrentTick - LastTick);
    LastTick = CurrentTick;
}

void VTime::SyncFrame()
{
    if (bLimitFPS)
    {
        while ((i32)GetTicks() - LastTick < MsFrameLimit)
        {
            VLN_PAUSE();
        }
    }
}

}