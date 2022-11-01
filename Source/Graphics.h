#ifndef GRAPHICS_H_

/* TODO(sean):
    - Backbuffer
    - Force inline define
    - MapRGB as macro
    - Maybe some inline assembly like for memsetq
 */

#include <stdio.h> // TODO(sean): Log
#include <stdlib.h> // TODO(sean): My random
#include "SDL.h"
#include "Window.h"
#include "Surface.h"
#include "PixelFormat.h"

class VGraphics
{
    VSurface VideoSurface;
    VPixelFormat PixelFormat;

public:
    void StartUp()
    {
        SDL_Surface* SDLSurface = SDL_GetWindowSurface(Window.GetWindow());
        VideoSurface.SetPlatformSurface(SDLSurface);
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
        i32 Pitch;
        VideoSurface.Lock(VideoBuffer, Pitch);
        {
            for (i32f Y = 0; Y < VideoSurface.GetHeight(); ++Y)
            {
                for (i32f X = 0; X < VideoSurface.GetWidth(); ++X)
                {
                    VideoBuffer[X] = MapRGB(rand() % 256, rand() % 256, rand() % 256);
                }
                VideoBuffer += Pitch;
            }
        }
        VideoSurface.Unlock();

        SDL_UpdateWindowSurface(Window.GetWindow());
    }

    const VPixelFormat& GetPixelFormat() const
    {
        return PixelFormat;
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