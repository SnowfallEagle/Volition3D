#ifndef SURFACE_H_

#include "SDL.h"
#include "Types.h"

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
    void Unlock()
    {
        if (SDL_MUSTLOCK(SDLSurface))
            SDL_UnlockSurface(SDLSurface);
    }

    void SetPlatformSurface(SDL_Surface* InSDLSurface) { SDLSurface = InSDLSurface; }
    SDL_Surface* GetPlatformSurface() { return SDLSurface; }
    void ToggleOwn(b32 bInOwn) { bOwn = bInOwn; }

    i32 GetWidth() const { return SDLSurface->w; }
    i32 GetHeight() const { return SDLSurface->h; }
};


#define SURFACE_H_
#endif