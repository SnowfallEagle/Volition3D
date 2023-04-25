#pragma once

#include "Engine/GameCore/Entity.h"

namespace Volition
{

class VTerrain : public VEntity
{
public:
    virtual void Update(f32 DeltaTime) override { } // @INCOMPLETE: Move a bit

    VLN_FINLINE void GenerateTerrain(const char* HeightMap, const char* Texture, f32 Size = 1000.0f, f32 Height = 1000.0f)
    {
        Mesh->GenerateTerrain(HeightMap, Texture, Size, Height);
    }
};

}