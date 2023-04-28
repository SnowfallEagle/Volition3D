#include "Engine/Core/Time.h"
#include "Engine/World/Terrain.h"

namespace Volition
{

void VTerrain::Update(f32 DeltaTime)
{
    Mesh->TransformModelToWorld();

    if (bBreathingEffect)
    {
        for (i32f I = 0; I < Mesh->NumVtx; ++I)
        {
            Mesh->TransVtxList[I].Y += Amplitude * Math.Sin((f32)Time.GetTicks() * Frequency * (f32)I * IndexMultiplier);
        }
    }
}

}
