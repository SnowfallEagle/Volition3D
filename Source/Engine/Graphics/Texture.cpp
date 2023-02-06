#include "Engine/Graphics/Renderer.h"
#include "Engine/Graphics/Texture.h"

void VTexture::LoadBMP(const char* Path)
{
    Surfaces.Resize(Renderer.GetRenderSpec().MaxMipMappingLevel);

    Surfaces[0].Load(Path);

    GenerateMipMaps();

    for (i32f I = 0; I < NumMipMaps; ++I)
    {
        u32* DummyBuffer;
        i32 DummyPitch;

        Surfaces[I].Lock(DummyBuffer, DummyPitch);
    }
}

void VTexture::Destroy()
{
    for (i32f I = 0; I < NumMipMaps; ++I)
    {
        Surfaces[I].Unlock();
        Surfaces[I].Destroy();
    }
}

const VSurface& VTexture::Get(i32 MipMapLevel) const
{
    if (MipMapLevel < 0)
    {
        MipMapLevel = 0;
    }
    else if (MipMapLevel >= NumMipMaps)
    {
        MipMapLevel = NumMipMaps - 1;
    }

    return Surfaces[MipMapLevel];
}


void VTexture::GenerateMipMaps()
{
    // TODO(sean)
    NumMipMaps = 1;
}