#include "Core/Platform.h"

#if VL_IMPL_SDL

#include "Core/Assert.h"
#include "Graphics/Renderer.h"
#include "Graphics/Impl/Surface_SDL.h"

#define SHOULD_LOCK_SDL_SURFACE 0

void VSurface::Create(SDL_Surface* InSDLSurface)
{
    ASSERT(InSDLSurface);
    Destroy();

    SDLSurface = InSDLSurface;
    Width = InSDLSurface->w;
    Height = InSDLSurface->h;
}

void VSurface::Create(i32 InWidth, i32 InHeight)
{
    Destroy();

    SDL_Surface* PlatformSurface = SDL_CreateRGBSurfaceWithFormat(
        0, InWidth, InHeight, Renderer.BitsPerPixel,
        Renderer.SDLPixelFormatEnum
    );
    ASSERT(PlatformSurface);

    SDLSurface = PlatformSurface;
    Width = SDLSurface->w;
    Height = SDLSurface->h;
}

void VSurface::Load(const char* Path)
{
    Destroy();

    SDL_Surface* Temp = SDL_LoadBMP(Path);
    ASSERT(Temp);

    SDL_Surface* Converted = SDL_ConvertSurface(
        Temp, Renderer.SDLPixelFormat, 0
    );
    ASSERT(Converted);
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

void VSurface::Lock(u32*& OutBuffer, i32& OutPitch)
{
    ASSERT(!bLocked);

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
    ASSERT(bLocked);
#if SHOULD_LOCK_SDL_SURFACE
    if (SDLSurface->locked) // Check if we don't have to unlock
        SDL_UnlockSurface(SDLSurface);
#endif // SHOULD_LOCK_SDL_SURFACE

    bLocked = false;
}

#endif VL_IMPL_SDL
