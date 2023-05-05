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

}