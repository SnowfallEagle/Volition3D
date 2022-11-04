#ifndef GRAPHICS_IMPL_GRAPHICS_SDL_H_

#include "SDL.h"
#include "SDL_ttf.h"
#include "Math/Rect.h"
#include "Core/Platform.h"
#include "Graphics/Impl/Surface_SDL.h"
#include "Graphics/Impl/IGraphics.h"

class VGraphics final : public IGraphics
{
    SDL_PixelFormat* SDLPixelFormat;
    u32 SDLPixelFormatEnum;

    TTF_Font* Font;
    i32 FontCharWidth; // In pixels
    i32 FontCharHeight;

public:
    void StartUp();
    void ShutDown();

    void PrepareToRender();
    void RenderAndFlip();

    FINLINE void DrawSurface(VSurface* Surface, VRelRectI* Source, VRelRectI* Dest)
    {
        BlitScaled(Surface, BackSurface, Source, Dest);
    }
    void DrawText(i32 X, i32 Y, u32 Color, const char* Format, ...);

    static FINLINE void BlitSurface(VSurface* Source, VSurface* Dest, VRelRectI* SourceRect, VRelRectI* DestRect)
    {
        SDL_BlitSurface(Source->SDLSurface, (SDL_Rect*)SourceRect, Dest->SDLSurface, (SDL_Rect*)DestRect);
    }
    static FINLINE void BlitScaled(VSurface* Source, VSurface* Dest, VRelRectI* SourceRect, VRelRectI* DestRect)
    {
        SDL_BlitScaled(Source->SDLSurface, (SDL_Rect*)SourceRect, Dest->SDLSurface, (SDL_Rect*)DestRect);
    }
    static FINLINE void FillRect(VSurface* Dest, VRelRectI* Rect, u32 Color)
    {
        SDL_FillRect(Dest->SDLSurface, (SDL_Rect*)Rect, Color);
    }

private:
    void Flip();

    friend class VSurface;
};

#define GRAPHICS_IMPL_GRAPHICS_SDL_H_
#endif
