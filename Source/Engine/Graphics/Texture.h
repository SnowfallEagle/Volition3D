#pragma once

#include "Engine/Core/Containers/SparseArray.h"
#include "Engine/Graphics/Surface.h"

class VTexture
{
    TSparseArray<VSurface> Surfaces;
    i32 NumMipMaps;

public:
    void LoadBMP(const char* Path);
    void Destroy();

    const VSurface& Get(i32 MipMapLevel) const;

private:
    void GenerateMipMaps();
};