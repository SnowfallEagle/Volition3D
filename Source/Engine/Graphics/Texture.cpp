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
    i32 MaxMipMappingLevel = Renderer.GetRenderSpec().MaxMipMappingLevel;
    NumMipMaps = 1;

    for (i32f I = 1; I < MaxMipMappingLevel; ++I, ++NumMipMaps)
    {
        i32 PrevSize = Surfaces[I - 1].GetWidth();
        i32 CurrentSize = PrevSize / 2;

        if (CurrentSize <= 0)
        {
            break;
        }

        Surfaces[I].Create(CurrentSize, CurrentSize);

        u32* CurrentBuffer;
        i32 CurrentPitch;
        Surfaces[I].Lock(CurrentBuffer, CurrentPitch);

        u32* PrevBuffer;
        i32 PrevPitch;
        Surfaces[I - 1].Lock(PrevBuffer, PrevPitch);

        for (i32f Y = 0; Y < CurrentSize; ++Y)
        {
            for (i32f X = 0; X < CurrentSize; ++X)
            {
                i32 PrevPixelY0 = (Y * 2) * PrevPitch;
                i32 PrevPixelY1 = PrevPixelY0 + PrevPitch;
                if (PrevPixelY1 > (PrevSize * PrevPitch))
                {
                    PrevPixelY1 = PrevPixelY0;
                }

                i32 PrevPixelX0 = (X * 2);
                i32 PrevPixelX1 = PrevPixelX0 + 1;
                if (PrevPixelX1 > PrevSize)
                {
                    PrevPixelX1 = PrevPixelX0;
                }

                VColorARGB PrevPixels[4] = {
                    PrevBuffer[PrevPixelY0 + PrevPixelX0],
                    PrevBuffer[PrevPixelY0 + PrevPixelX1],
                    PrevBuffer[PrevPixelY1 + PrevPixelX0],
                    PrevBuffer[PrevPixelY1 + PrevPixelX1]
                };

                VColorARGB FilteredPixel;
                FilteredPixel.R = (
                    PrevPixels[0].R +
                    PrevPixels[1].R +
                    PrevPixels[2].R +
                    PrevPixels[3].R
                ) >> 2; // Divide by 4

                FilteredPixel.G = (
                    PrevPixels[0].G +
                    PrevPixels[1].G +
                    PrevPixels[2].G +
                    PrevPixels[3].G
                ) >> 2; // Divide by 4

                FilteredPixel.B = (
                    PrevPixels[0].B +
                    PrevPixels[1].B +
                    PrevPixels[2].B +
                    PrevPixels[3].B
                ) >> 2; // Divide by 4

                CurrentBuffer[Y * CurrentPitch + X] = FilteredPixel;
            }
        }

        Surfaces[I - 1].Unlock();
        Surfaces[I].Unlock();
    }
}