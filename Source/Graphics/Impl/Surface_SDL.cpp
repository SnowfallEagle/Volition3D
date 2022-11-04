#include "Core/Platform.h"

#if VL_IMPL_SDL

#include "Core/Assert.h"
#include "Graphics/Graphics.h"
#include "Graphics/Impl/Surface_SDL.h"

#define SHOULD_LOCK_SDL_SURFACE 0

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
     */
#if SHOULD_LOCK_SDL_SURFACE
    if (SDL_MUSTLOCK(SDLSurface))
        SDL_LockSurface(SDLSurface);
#endif // SHOULD_LOCK_SDL_SURFACE

    OutBuffer = Buffer = (u32*)SDLSurface->pixels;
    OutPitch = Pitch = SDLSurface->pitch >> 2; // Divide by 4 (Bytes per pixel)

    bLocked = true;
}

void VSurface::Unlock()
{
#if SHOULD_LOCK_SDL_SURFACE
    if (SDLSurface->locked) // Check if we don't have to unlock
        SDL_UnlockSurface(SDLSurface);
#endif // SHOULD_LOCK_SDL_SURFACE

    bLocked = false;
}

#endif VL_IMPL_SDL
