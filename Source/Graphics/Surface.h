#ifndef GRAPHICS_SURFACE_H_

#include "SDL.h"
#include "Core/Types.h"
#include "Core/Platform.h"

class VSurface
{
    SDL_Surface* SDLSurface;
    b32 bOwn;

public:
    VSurface(SDL_Surface* InSDLSurface = nullptr, b32 bInOwn = true)
        : SDLSurface(InSDLSurface), bOwn(bInOwn)
    {
    }
    ~VSurface()
    {
        if (bOwn && SDLSurface)
            SDL_FreeSurface(SDLSurface);
    }

    void Lock(u32*& OutVideoBuffer, i32& OutPitchInPixels);
    FINLINE void Unlock()
    {
        if (SDL_MUSTLOCK(SDLSurface))
            SDL_UnlockSurface(SDLSurface);
    }

    FINLINE void SetPlatformSurface(SDL_Surface* InSDLSurface)
    {
        SDLSurface = InSDLSurface;
    }
    FINLINE SDL_Surface* GetPlatformSurface()
    {
        return SDLSurface;
    }
    FINLINE void ToggleOwn(b32 bInOwn)
    {
        bOwn = bInOwn;
    }

    FINLINE i32 GetWidth() const 
    {
        return SDLSurface->w;
    }
    FINLINE i32 GetHeight() const
    {
        return SDLSurface->h;
    }
};


#define GRAPHICS_SURFACE_H_
#endif