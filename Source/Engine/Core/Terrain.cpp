#include "Engine/Core/Time.h"
#include "Engine/Core/Terrain.h"

namespace Volition
{

void VTerrain::Update(f32 DeltaTime)
{
    Mesh->TransformModelToWorld();

    for (i32f I = 0; I < Mesh->NumVtx; ++I)
    {
        static constexpr f32 Amplitude = 10.0f;
        static constexpr f32 Frequency = 0.0005f;
        static constexpr f32 IndexMultiplier = 0.5f;

        Mesh->TransVtxList[I].Y += Amplitude * Math.Sin((f32)Time.GetTicks() * Frequency * (f32)I * IndexMultiplier);
    }
}

}
