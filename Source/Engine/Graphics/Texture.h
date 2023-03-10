#pragma once

#include "Engine/Core/Containers/Array.h"
#include "Engine/Graphics/Surface.h"

namespace Volition
{

class VTexture
{
    TArray<VSurface> Surfaces;
    i32 NumMipMaps;

public:
    void LoadBMP(const char* Path);
    void Destroy();

    const VSurface& Get(i32 MipMapLevel) const;

private:
    void GenerateMipMaps();
};

}