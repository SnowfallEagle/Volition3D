#ifndef GRAPHICS_H_

#include <stdio.h> // TODO(sean): Log
#include <stdlib.h> // TODO(sean): My random
#include <string.h>
#include "SDL.h"
#include "Core/Window.h"
#include "Core/Platform.h"
#include "Core/Assert.h"
#include "Graphics/Surface.h"
#include "Graphics/PixelFormat.h"

#define _RGBA32(A, R, G, B) ( ((A) << 24) | ((R) << 16) | ((G) << 8) | (B) )
#define _RGB32(R, G, B) ( 0xFF000000 | ((R) << 16) | ((G) << 8) | (B) )

class VGraphics
{
    VSurface VideoSurface;
    VSurface BackSurface;
    VPixelFormat PixelFormat;

    u32* BackBuffer;
    i32 BackPitchInPixels;

public:
    void StartUp()
    {
        // Set up video surface
        SDL_Surface* SDLSurface = SDL_GetWindowSurface(Window.GetWindow());
        ASSERT(SDLSurface);
        VideoSurface.SetPlatformSurface(SDLSurface);
        VideoSurface.ToggleOwn(false);

        // Pixel format
        SDL_PixelFormat* SDLFormat = VideoSurface.GetPlatformSurface()->format;
        PixelFormat = {
            SDLFormat->Aloss, SDLFormat->Rloss, SDLFormat->Gloss, SDLFormat->Bloss,
            SDLFormat->Ashift, SDLFormat->Rshift, SDLFormat->Gshift, SDLFormat->Bshift,
            SDLFormat->Amask, SDLFormat->Rmask, SDLFormat->Gmask, SDLFormat->Bmask,
            SDLFormat->BytesPerPixel, SDLFormat->BitsPerPixel
        };

        // Back surface
        SDL_Surface* Temp = SDL_CreateRGBSurfaceWithFormat(
            0, VideoSurface.GetWidth(), VideoSurface.GetHeight(),
            PixelFormat.BitsPerPixel, VideoSurface.GetPlatformSurface()->format->format
        );
        ASSERT(Temp);

        SDL_Surface* Converted = SDL_ConvertSurface(Temp, VideoSurface.GetPlatformSurface()->format, 0);
        ASSERT(Converted);
        SDL_FreeSurface(Temp);

        BackSurface.SetPlatformSurface(Converted);

        // Other
        BackBuffer = nullptr;
        BackPitchInPixels = 0;
    }
    void ShutDown()
    {
    }

    void PrepareToRender()
    {
        SDL_FillRect(BackSurface.GetPlatformSurface(), nullptr, _RGB32(0x00, 0x00, 0x00));
        BackSurface.Lock(BackBuffer, BackPitchInPixels);
    }
    void Render()
    {
        i32f Height = BackSurface.GetHeight();
        i32f Width = BackSurface.GetWidth();
        u32* Buffer = BackBuffer;
        for (i32f Y = 0; Y < Height; ++Y)
        {
            for (i32f X = 0; X < Width; ++X)
            {
                Buffer[X] = MapRGB(rand() % 256, rand() % 256, rand() % 256);
            }
            Buffer += BackPitchInPixels;
        }

        Flip();
    }

    FINLINE u32 MapRGB(u8 R, u8 G, u8 B)
    {
        return
            (R >> PixelFormat.RedLoss)   << PixelFormat.RedShift   |
            (G >> PixelFormat.GreenLoss) << PixelFormat.GreenShift |
            (B >> PixelFormat.BlueLoss)  << PixelFormat.BlueShift  |
            PixelFormat.AlphaMask;
    }

    FINLINE const VPixelFormat& GetPixelFormat() const
    {
        return PixelFormat;
    }
    FINLINE u32* GetVideoBuffer()
    {
        return BackBuffer;
    }
    FINLINE i32 GetPitch()
    {
        return BackPitchInPixels;
    }

private:
    void Flip()
    {
        u32* VideoBuffer;
        i32 Pitch;
        VideoSurface.Lock(VideoBuffer, Pitch);
        {
            i32f Height = VideoSurface.GetHeight();
            i32f BytesWidth = VideoSurface.GetWidth() * PixelFormat.BytesPerPixel;

            for (i32f Y = 0; Y < Height; ++Y)
            {
                memcpy(VideoBuffer, BackBuffer, BytesWidth);
                VideoBuffer += Pitch;
                BackBuffer += BackPitchInPixels;
            }
        }
        VideoSurface.Unlock();
        BackSurface.Unlock();

        SDL_UpdateWindowSurface(Window.GetWindow());
    }
};

extern VGraphics Graphics;

#define GRAPHICS_H_
#endif
