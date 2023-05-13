#pragma once

#include "Common/Types/Array.h"
#include "Engine/Graphics/Rendering/Surface.h"

namespace Volition
{

class VTexture
{
    TArray<VSurface> Surfaces;
    i32 NumMipMaps;

public:
    void Load(const char* Path, const VVector3& ColorCorrection = { 1.0f, 1.0f, 1.0f }, i32 MaxMipMaps = -1);
    void Destroy();

    const VSurface& Get(i32 MipMaps) const;

private:
    void GenerateMipMaps(i32 MaxMipMaps);
};

}