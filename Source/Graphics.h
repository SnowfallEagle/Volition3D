#ifndef GRAPHICS_H_

#include "SDL.h"
#include "Window.h"

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

    void Lock(u32*& OutVideoBuffer, i32& OutPitch)
    {
        if (SDL_MUSTLOCK(SDLSurface))
            SDL_LockSurface(SDLSurface);

        OutVideoBuffer = (u32*)SDLSurface->pixels;
        OutPitch = SDLSurface->pitch;
    }
    void Unlock()
    {
        if (SDL_MUSTLOCK(SDLSurface))
            SDL_UnlockSurface(SDLSurface);
    }

    void SetPlatformSurface(SDL_Surface* InSDLSurface) { SDLSurface = InSDLSurface; }
    SDL_Surface* GetPlatformSurface() { return SDLSurface; }
    void ToggleOwn(b32 bInOwn) { bOwn = bInOwn; }

    i32 GetBytesPerPixel() const { return SDLSurface->format->BytesPerPixel; }
    i32 GetBitsPerPixel() const { return SDLSurface->format->BitsPerPixel; }
    i32 GetWidth() const { return SDLSurface->w; }
    i32 GetHeight() const { return SDLSurface->h; }
};

class VGraphics
{
    VSurface VideoSurface;

public:
    void StartUp()
    {
        VideoSurface.SetPlatformSurface(SDL_GetWindowSurface(Window.GetWindow()));
        VideoSurface.ToggleOwn(false);
    }
    void ShutDown()
    {
    }

    void PrepareToRender()
    {
        SDL_FillRect(VideoSurface.GetPlatformSurface(), nullptr, SDL_MapRGB(VideoSurface.GetPlatformSurface()->format, 0x00, 0x00, 0x00));
    }
    void Render()
    {
        SDL_UpdateWindowSurface(Window.GetWindow());
    }
};

extern VGraphics Graphics;

#define GRAPHICS_H_
#endif