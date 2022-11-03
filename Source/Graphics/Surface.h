#ifndef GRAPHICS_SURFACE_H_

#include "SDL.h"
#include "Core/Types.h"
#include "Core/Platform.h"
#include "Graphics/PixelFormat.h"

class VSurface
{
    SDL_Surface* SDLSurface = nullptr;
    u32* Buffer = nullptr;
    i32 Pitch = 0; // In pixels
    i32 Width = 0;
    i32 Height = 0;
    b32 bLocked = false;

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

    FINLINE i32 GetWidth() const 
    {
        return Width;
    }
    FINLINE i32 GetHeight() const
    {
        return Height;
    }

private:
    VSurface() = default;
    VSurface(SDL_Surface* InSDLSurface, i32 InWidth, i32 InHeight)
        : SDLSurface(InSDLSurface), Width(InWidth), Height(InHeight) {}

    friend class VGraphics;
};


#define GRAPHICS_SURFACE_H_
#endif