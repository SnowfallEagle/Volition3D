#include "SDL_image.h"
#include "Engine/Graphics/Rendering/Renderer.h"
#include "Engine/Graphics/Rendering/Surface.h"

namespace Volition
{

void VSurface::Create(SDL_Surface* InSDLSurface)
{
    VLN_ASSERT(InSDLSurface);
    Destroy();

    SDLSurface = InSDLSurface;
    Width = InSDLSurface->w;
    Height = InSDLSurface->h;
}

void VSurface::Create(i32 InWidth, i32 InHeight)
{
    Destroy();

    SDL_Surface* PlatformSurface = SDL_CreateRGBSurfaceWithFormat(
        0, InWidth, InHeight, Config.RenderSpec.BitsPerPixel,
        Config.RenderSpec.SDLPixelFormatEnum
    );
    VLN_ASSERT(PlatformSurface);

    SDLSurface = PlatformSurface;
    Width = SDLSurface->w;
    Height = SDLSurface->h;
}

void VSurface::Load(const char* Path)
{
    Destroy();

    SDL_Surface* Temp = IMG_Load(Path);
    VLN_ASSERT(Temp);

    SDL_Surface* Converted = SDL_ConvertSurface(
        Temp, Config.RenderSpec.SDLPixelFormat, 0
    );
    VLN_ASSERT(Converted);
    SDL_FreeSurface(Temp);

    Create(Converted);
}

void VSurface::Destroy()
{
    if (SDLSurface)
    {
        SDL_FreeSurface(SDLSurface);
        SDLSurface = nullptr;
    }

    Buffer = nullptr;
    Pitch = 0;
    bLocked = false;
    Width = 0;
    Height = 0;
}

void VSurface::CorrectColors(const VVector3& ColorCorrection)
{
    u32* Buffer;
    i32 Pitch;
    Lock(Buffer, Pitch);

    for (i32f Y = 0; Y < Height; ++Y)
    {
        for (i32f X = 0; X < Width; ++X)
        {
            VColorARGB Pixel = Buffer[X];

            i32 Colors[3] = {
                (i32)( (((f32)Pixel.R / 255.0f) * ColorCorrection.X) * 255.0f + 0.5f ),
                (i32)( (((f32)Pixel.G / 255.0f) * ColorCorrection.Y) * 255.0f + 0.5f ),
                (i32)( (((f32)Pixel.B / 255.0f) * ColorCorrection.Z) * 255.0f + 0.5f )
            };

            for (i32f ColorIndex = 0; ColorIndex < 3; ++ColorIndex)
            {
                Colors[ColorIndex] = VLN_MIN(VLN_MAX(Colors[ColorIndex], 0), 255);
            }

            Buffer[X] = MAP_ARGB32(
                Pixel.A,
                Colors[0],
                Colors[1],
                Colors[2]
            );
        }

        Buffer += Pitch;
    }

    Unlock();
}

}