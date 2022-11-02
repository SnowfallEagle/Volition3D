#ifndef GRAPHICS_GRAPHICS_H_

#include <stdlib.h> // TODO(sean): math::Random
#include <string.h> // TODO(sean): mem::MemSet, mem::MemCopy()
#include "SDL.h"
#include "Core/Window.h"
#include "Core/Platform.h"
#include "Core/Assert.h"
#include "Graphics/Surface.h"
#include "Graphics/PixelFormat.h"

// Macroses for ABGR8888(High bit -> Low bit) format
#define _ALPHA_MASK (0xFF << 24)
#define _RGBA32(A, R, G, B) ( ((A) << 24) | ((R) << 16) | ((G) << 8) | (B) )
#define _RGB32(R, G, B) ( _ALPHA_MASK | ((R) << 16) | ((G) << 8) | (B) )

class VGraphics
{
    VSurface VideoSurface;
    VSurface BackSurface;
    VPixelFormat PixelFormat;

    u32* BackBuffer;
    i32 BackPitchInPixels;

public:
    void StartUp();
    void ShutDown();

    void PrepareToRender();
    void Render();

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
    void Flip();
};

extern VGraphics Graphics;

#define GRAPHICS_GRAPHICS_H_
#endif
