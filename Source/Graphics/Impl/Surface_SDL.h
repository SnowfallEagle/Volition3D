#ifndef GRAPHICS_IMPL_SURFACE_SDL_H_

#include "SDL.h"
#include "Graphics/Impl/ISurface.h"

class VSurface final : public ISurface
{
    SDL_Surface* SDLSurface = nullptr;

public:
    static VSurface* Create(SDL_Surface* InSDLSurface);
    static VSurface* Create(i32 InWidth, i32 InHeight);
    static VSurface* Load(const char* Path);
    void Destroy();

    void Lock(u32*& OutBuffer, i32& OutPitch);
    void Unlock();

    FINLINE void EnableColorKey()
    {
        SDL_SetColorKey(SDLSurface, SDL_TRUE, 0);
    }

private:
    VSurface() = default;
    VSurface(SDL_Surface* InSDLSurface, i32 InWidth, i32 InHeight)
        : ISurface(InWidth, InHeight), SDLSurface(InSDLSurface)
    {
    }

    friend class VGraphics;
};

#define GRAPHICS_IMPL_SURFACE_SDL_H_
#endif