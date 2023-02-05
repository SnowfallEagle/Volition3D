#include "Engine/Graphics/Renderer.h"
#include "Engine/Graphics/Texture.h"

void VTexture::LoadBMP(const char* Path)
{
    NumMipMaps = Renderer.GetRenderSpec().MaxMipMappingLevel;
    Surfaces.Resize(NumMipMaps);

    Surfaces[0].Load(Path);
    GenerateMipMaps();
}

void VTexture::Destroy()
{
    for (i32f I = 0; I < NumMipMaps; ++I)
    {
        Surfaces[I].Destroy();
    }
}

const VSurface& VTexture::Get(i32 MipMapLevel) const
{
    if (MipMapLevel < 0)
    {
        MipMapLevel = 0;
    }
    else if (MipMapLevel > NumMipMaps)
    {
        MipMapLevel = NumMipMaps;
    }

    return Surfaces[MipMapLevel];
}


void VTexture::GenerateMipMaps()
{
    // TODO(sean)
}