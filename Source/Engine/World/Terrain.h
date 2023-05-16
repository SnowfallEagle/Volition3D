#pragma once

#include "Engine/World/Entity.h"

namespace Volition
{

// @TODO: Maybe use only Mesh in World???
class VTerrain : public VEntity
{
public:
    void GenerateTerrain(const char* HeightMap, const char* Texture, f32 Size = 15000.0f, f32 Height = 1000.0f, EShadeMode ShadeMode = EShadeMode::Gouraud);
};

}