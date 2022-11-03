/* TODO(sean)
    - Emulated funcs
 */

#ifndef GRAPHICS_GRAPHICS_H_

#include <stdlib.h> // TODO(sean): math::Random
#include <string.h> // TODO(sean): mem::MemSet, mem::MemCopy()
#include "SDL.h"
#include "SDL_ttf.h"
#include "Core/Window.h"
#include "Core/Platform.h"
#include "Core/Assert.h"
#include "Math/Rect.h"
#include "Graphics/Surface.h"
#include "Graphics/PixelFormat.h"

// Macroses for fast mapping ARGB32/XRGB32 format
#define _RGBA32(A, R, G, B) ( ((A) << 24) | ((R) << 16) | ((G) << 8) | (B) )
#define _RGB32(R, G, B) ( ((R) << 16) | ((G) << 8) | (B) ) // Alpha = 0

#define _ALPHA_MASK (0xFF << 24)
#define _RED_MASK (0xFF << 16)
#define _GREEN_MASK (0xFF << 8)
#define _BLUE_MASK (0xFF)

class VGraphics
{
public:
    static constexpr i32f BytesPerPixel = 4;
    static constexpr i32f BitsPerPixel = 32;

private:
    VSurface* VideoSurface;
    VSurface* BackSurface;

    i32 ScreenWidth;
    i32 ScreenHeight;

    TTF_Font* Font;
    i32 FontCharWidth; // In pixels
    i32 FontCharHeight;

public:
    void StartUp();
    void ShutDown();

    void PrepareToRender();
    void Render();

    FINLINE void DrawSurface(VSurface* Surface, VRelativeRectI* Source, VRelativeRectI* Dest)
    {
        BlitScaled(Surface, BackSurface, Source, Dest);
    }
    void DrawText(const char* Text, i32 X, i32 Y, u32 Color);

    static FINLINE void BlitSurface(VSurface* Source, VSurface* Dest, VRelativeRectI* SourceRect, VRelativeRectI* DestRect)
    {
        SDL_BlitSurface(Source->SDLSurface, (SDL_Rect*)SourceRect, Dest->SDLSurface, (SDL_Rect*)DestRect);
    }
    static FINLINE void BlitScaled(VSurface* Source, VSurface* Dest, VRelativeRectI* SourceRect, VRelativeRectI* DestRect)
    {
        SDL_BlitScaled(Source->SDLSurface, (SDL_Rect*)SourceRect, Dest->SDLSurface, (SDL_Rect*)DestRect);
    }
    static FINLINE void FillRect(VSurface* Dest, VRelativeRectI* Rect, u32 Color)
    {
        SDL_FillRect(Dest->SDLSurface, (SDL_Rect*)Rect, Color);
    }

    // Slow mapping RGB functions
    static FINLINE u32 MapRGB(u8 R, u8 G, u8 B)
    {
        return
            (R >> PixelFormat.RedLoss)   << PixelFormat.RedShift   |
            (G >> PixelFormat.GreenLoss) << PixelFormat.GreenShift |
            (B >> PixelFormat.BlueLoss)  << PixelFormat.BlueShift  |
            PixelFormat.AlphaMask;
    }
    static FINLINE u32 MapARGB(u8 A, u8 R, u8 G, u8 B)
    {
        return
            (A >> PixelFormat.AlphaLoss) << PixelFormat.AlphaShift |
            (R >> PixelFormat.RedLoss)   << PixelFormat.RedShift   |
            (G >> PixelFormat.GreenLoss) << PixelFormat.GreenShift |
            (B >> PixelFormat.BlueLoss)  << PixelFormat.BlueShift  |
            PixelFormat.AlphaMask;
    }

private:
    void Flip();

    friend class VSurface;
};

extern VGraphics Graphics;

#define GRAPHICS_GRAPHICS_H_
#endif
