#include "Engine/Core/Time.h"
#include "Engine/World/Terrain.h"

namespace Volition
{

void VTerrain::Update(f32 DeltaTime)
{
    Mesh->TransformModelToWorld();

    if (bBreathingEffect)
    {
        for (i32f i = 0; i < Mesh->NumVtx; ++i)
        {
            Mesh->TransVtxList[i].Y += Amplitude * Math.Sin((f32)Time.GetTicks() * Frequency * (f32)i * IndexMultiplier);
        }
    }
}

}
