#include "Engine/Graphics/Rendering/Renderer.h"
#include "Engine/Graphics/Rendering/Texture.h"

namespace Volition
{

void VTexture::Load(const char* Path, const VVector3& ColorCorrection, i32 MaxMipMaps)
{
    if (MaxMipMaps <= 0)
    {
        MaxMipMaps = Config.RenderSpec.MaxMipMaps;
    }

    Surfaces.Resize(MaxMipMaps);
    Surfaces[0].Load(Path);

    Surfaces[0].CorrectColorsSlow(ColorCorrection);
    GenerateMipMaps(MaxMipMaps);

    for (i32f i = 0; i < NumMipMaps; ++i)
    {
        u32* DummyBuffer;
        i32 DummyPitch;

        Surfaces[i].Lock(DummyBuffer, DummyPitch);
    }
}

void VTexture::Destroy()
{
    for (i32f i = 0; i < NumMipMaps; ++i)
    {
        Surfaces[i].Unlock();
        Surfaces[i].Destroy();
    }
}

const VSurface& VTexture::Get(i32 MipMaps) const
{
    if (MipMaps < 0)
    {
        MipMaps = 0;
    }
    else if (MipMaps >= NumMipMaps)
    {
        MipMaps = NumMipMaps - 1;
    }

    return Surfaces[MipMaps];
}


void VTexture::GenerateMipMaps(i32 MaxMipMaps)
{
    NumMipMaps = 1;

    for (i32f i = 1; i < MaxMipMaps; ++i, ++NumMipMaps)
    {
        const VVector2i PrevSize = { Surfaces[i - 1].GetWidth(), Surfaces[i - 1].GetHeight() };
        const VVector2i CurrentSize = { PrevSize.X / 2, PrevSize.Y / 2 };

        if (CurrentSize.X <= 0 || CurrentSize.Y <= 0)
        {
            break;
        }

        Surfaces[i].Create(CurrentSize.X, CurrentSize.Y);

        u32* CurrentBuffer;
        i32 CurrentPitch;
        Surfaces[i].Lock(CurrentBuffer, CurrentPitch);

        u32* PrevBuffer;
        i32 PrevPitch;
        Surfaces[i - 1].Lock(PrevBuffer, PrevPitch);

        for (i32f Y = 0; Y < CurrentSize.Y; ++Y)
        {
            for (i32f X = 0; X < CurrentSize.X; ++X)
            {
                const i32 PrevPixelY0 = (Y * 2) * PrevPitch;
                i32 PrevPixelY1 = PrevPixelY0 + PrevPitch;
                if (PrevPixelY1 > (PrevSize.Y * PrevPitch))
                {
                    PrevPixelY1 = PrevPixelY0;
                }

                const i32 PrevPixelX0 = (X * 2);
                i32 PrevPixelX1 = PrevPixelX0 + 1;
                if (PrevPixelX1 > PrevSize.X)
                {
                    PrevPixelX1 = PrevPixelX0;
                }

                const VColorARGB PrevPixels[4] = {
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

        Surfaces[i - 1].Unlock();
        Surfaces[i].Unlock();
    }
}

}