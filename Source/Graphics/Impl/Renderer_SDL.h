#pragma once

#include "SDL.h"
#include "SDL_ttf.h"
#include "Math/Rect.h"
#include "Core/Platform.h"
#include "Graphics/Impl/Surface_SDL.h"
#include "Graphics/Impl/IRenderer.h"

class VRenderer final : public IRenderer
{
    VSurface VideoSurface;

    SDL_PixelFormat* SDLPixelFormat;
    u32 SDLPixelFormatEnum;

    TTF_Font* Font;
    i32 FontCharWidth; // In pixels
    i32 FontCharHeight;

public:
    void StartUp();
    void ShutDown();

    virtual void DrawText(i32 X, i32 Y, u32 Color, const char* Format, ...) override;

private:
    void Flip();

    friend class VSurface;
};
