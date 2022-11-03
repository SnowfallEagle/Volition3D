#include "Core/Assert.h"
#include "Graphics/Graphics.h"
#include "Graphics/Surface.h"

VSurface* VSurface::Create(SDL_Surface* InSDLSurface)
{
    ASSERT(InSDLSurface);
    return new VSurface(InSDLSurface, InSDLSurface->w, InSDLSurface->h);
}

VSurface* VSurface::Create(i32 InWidth, i32 InHeight)
{
    SDL_Surface* SDLSurface = SDL_CreateRGBSurfaceWithFormat(
        0, InWidth, InHeight, Graphics.BitsPerPixel,
        Graphics.SDLPixelFormatEnum
    );
    ASSERT(SDLSurface);

    return new VSurface(SDLSurface, InWidth, InHeight);
}

VSurface* VSurface::Load(const char* Path)
{
    SDL_Surface* Temp = SDL_LoadBMP(Path);
    ASSERT(Temp);

    SDL_Surface* Converted = SDL_ConvertSurface(
        Temp, Graphics.SDLPixelFormat, 0
    );
    ASSERT(Converted);
    SDL_FreeSurface(Temp);

    return Create(Converted);
}

void VSurface::Destroy()
{
    if (SDLSurface)
        SDL_FreeSurface(SDLSurface);
}

void VSurface::Lock(u32*& OutBuffer, i32& OutPitch)
{
    ASSERT(bLocked);

    /**
       NOTE(sean): We don't have to lock surfaces since
       we don't use RLE, but in future we may will

    if (SDL_MUSTLOCK(SDLSurface))
    {
        SDL_LockSurface(SDLSurface);
        OutBuffer = Buffer = (u32*)SDLSurface->pixels;
        OutPitch = Pitch = SDLSurface->pitch >> 2; // Divide by 4 (Bytes per pixel)
    }
    else
    {
        OutBuffer = Buffer = (u32*)SDLSurface->pixels;
        OutPitch = Pitch = SDLSurface->pitch >> 2;
    }
    */

    OutBuffer = Buffer = (u32*)SDLSurface->pixels;
    OutPitch = Pitch = SDLSurface->pitch >> 2;

    bLocked = true;
}

void VSurface::Unlock()
{
    /*
    if (SDLSurface->locked) // Check if we don't have to lock/unlock
        SDL_UnlockSurface(SDLSurface);
    */

    bLocked = false;
}
