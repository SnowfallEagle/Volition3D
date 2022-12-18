#pragma once

#include "SDL.h"
#include "SDL_ttf.h"
#include "Math/Rect.h"
#include "Core/Platform.h"
#include "Graphics/Impl/Surface_SDL.h"
#include "Graphics/Impl/IRenderer.h"

class VSDLRenderer final : public IRenderer
{
    VSurface VideoSurface;

    SDL_PixelFormat* SDLPixelFormat;
    u32 SDLPixelFormatEnum;

    TTF_Font* Font;
    i32 FontCharWidth; // In pixels
    i32 FontCharHeight;

public:
    virtual void StartUp() override;
    virtual void ShutDown() override;

    virtual void DrawText(i32 X, i32 Y, VColorARGB Color, const char* Format, ...) override;

private:
    virtual void Flip() override;

    friend class VSDLSurface;
};

typedef VSDLRenderer VRenderer;
