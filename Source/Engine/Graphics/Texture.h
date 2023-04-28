#pragma once

#include "Engine/Core/Types/Array.h"
#include "Engine/Graphics/Surface.h"

namespace Volition
{

class VTexture
{
    TArray<VSurface> Surfaces;
    i32 NumMipMaps;

public:
    void LoadBMP(const char* Path, i32 MaxMipMaps = -1);
    void Destroy();

    const VSurface& Get(i32 MipMaps) const;

private:
    void GenerateMipMaps(i32 MaxMipMaps);
};

}