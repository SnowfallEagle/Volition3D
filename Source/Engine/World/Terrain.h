#pragma once

#include "Engine/World/Entity.h"

namespace Volition
{

class VTerrain : public VEntity
{
public:
    f32 Amplitude       = 0.0f;
    f32 Frequency       = 0.0f;
    f32 IndexMultiplier = 0.0f;

    b32 bBreathingEffect = false;

public:
    virtual void Update(f32 DeltaTime) override;

    VLN_FINLINE void GenerateTerrain(const char* HeightMap, const char* Texture, f32 Size = 15000.0f, f32 Height = 1000.0f)
    {
        Mesh->Attr |= EMeshAttr::TerrainMesh;
        Mesh->GenerateTerrain(HeightMap, Texture, Size, Height);
    }
};

}