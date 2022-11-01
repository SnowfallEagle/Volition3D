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

    i32 GetWidth() const { return SDLSurface->w; }
    i32 GetHeight() const { return SDLSurface->h; }
};

class VPixelFormat
{
public:
    u32 AlphaLoss, RedLoss, GreenLoss, BlueLoss;
    u32 AlphaShift, RedShift, GreenShift, BlueShift;
    u32 AlphaMask, RedMask, GreenMask, BlueMask;
    i32 BytesPerPixel, BitsPerPixel;
};

class VGraphics
{
    VSurface VideoSurface;
    VPixelFormat PixelFormat;

public:
    void StartUp()
    {
        VideoSurface.SetPlatformSurface(SDL_GetWindowSurface(Window.GetWindow()));
        VideoSurface.ToggleOwn(false);

        SDL_PixelFormat* SDLFormat = VideoSurface.GetPlatformSurface()->format;
        PixelFormat = {
            SDLFormat->Aloss, SDLFormat->Rloss, SDLFormat->Gloss, SDLFormat->Bloss,
            SDLFormat->Ashift, SDLFormat->Rshift, SDLFormat->Gshift, SDLFormat->Bshift,
            SDLFormat->Amask, SDLFormat->Rmask, SDLFormat->Gmask, SDLFormat->Bmask,
            SDLFormat->BytesPerPixel, SDLFormat->BitsPerPixel
        };
    }
    void ShutDown()
    {
    }

    void PrepareToRender()
    {
        SDL_FillRect(VideoSurface.GetPlatformSurface(), nullptr, MapRGB(0x00, 0x00, 0x00));
    }
    void Render()
    {
        u32* VideoBuffer;
        i32 PixelsPitch;
        u32 Pixel = MapRGB(255, 0, 0);
        VideoSurface.Lock(VideoBuffer, PixelsPitch);
        PixelsPitch /= PixelFormat.BytesPerPixel;
        {
            for (i32f Y = 0; Y < VideoSurface.GetHeight(); ++Y)
            {
                for (i32f X = 0; X < VideoSurface.GetWidth(); ++X)
                {
                    VideoBuffer[X] = Pixel;
                }
                VideoBuffer += PixelsPitch;
            }
        }
        VideoSurface.Unlock();

        SDL_UpdateWindowSurface(Window.GetWindow());
    }

    u32 MapRGB(u8 R, u8 G, u8 B)
    {
        return
            (R >> PixelFormat.RedLoss)   << PixelFormat.RedShift   |
            (G >> PixelFormat.GreenLoss) << PixelFormat.GreenShift |
            (B >> PixelFormat.BlueLoss)  << PixelFormat.BlueShift  |
            PixelFormat.AlphaMask;
    }
};

extern VGraphics Graphics;

#define GRAPHICS_H_
#endif