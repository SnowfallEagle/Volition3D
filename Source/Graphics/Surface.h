#ifndef GRAPHICS_SURFACE_H_

#include "SDL.h"
#include "Core/Types.h"
#include "Core/Platform.h"
#include "Graphics/PixelFormat.h"

class VSurface
{
    SDL_Surface* SDLSurface;

public:
    VSurface(SDL_Surface* InSDLSurface = nullptr) : SDLSurface(InSDLSurface)
    {
    }
    ~VSurface()
    {
        if (SDLSurface)
            SDL_FreeSurface(SDLSurface);
    }

    FINLINE void SetPlatformSurface(SDL_Surface* InSDLSurface)
    {
        SDLSurface = InSDLSurface;
    }
    FINLINE SDL_Surface* GetPlatformSurface()
    {
        return SDLSurface;
    }

    FINLINE void EnableColorKey()
    {
        SDL_SetColorKey(SDLSurface, SDL_TRUE, 0);
    }

    FINLINE u32* GetBuffer()
    {
        return (u32*)SDLSurface->pixels;
    }
    FINLINE i32 GetPitch()
    {
        return SDLSurface->pitch / PixelFormat.BytesPerPixel;
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