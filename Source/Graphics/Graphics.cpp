#include "Graphics/Graphics.h"

VPixelFormat PixelFormat;
VGraphics Graphics;

DEFINE_LOG_CHANNEL(hLogGraphics, "Graphics");

void VGraphics::StartUp()
{
    // Get window surface
    SDL_Surface* SDLSurface = SDL_GetWindowSurface(Window.SDLWindow);
    ASSERT(SDLSurface);

    // Init pixel format
    SDL_PixelFormat* SDLFormat = SDLSurface->format;
    PixelFormat = {
        SDLFormat->Aloss, SDLFormat->Rloss, SDLFormat->Gloss, SDLFormat->Bloss,
        SDLFormat->Ashift, SDLFormat->Rshift, SDLFormat->Gshift, SDLFormat->Bshift,
        SDLFormat->Amask, SDLFormat->Rmask, SDLFormat->Gmask, SDLFormat->Bmask,
        SDLFormat->BytesPerPixel, SDLFormat->BitsPerPixel,
        SDLFormat, SDLFormat->format
    };

    // Create video and back surfaces
    VideoSurface = VSurface::Create(SDLSurface);
    ScreenWidth = VideoSurface->Width;
    ScreenHeight = VideoSurface->Height;
    BackSurface = VSurface::Create(ScreenWidth, ScreenHeight);

    // Initialize TTF
    i32 Res = TTF_Init();
    ASSERT(Res == 0);

    static constexpr i32f CharsPerLine = 100;
    static constexpr f32 PointDivPixel = 3/4;
    Font = TTF_OpenFont("Font.ttf", (i32f)( (f32)(ScreenWidth/CharsPerLine) * PointDivPixel) );
    ASSERT(Font);

    // Log
    VL_NOTE(hLogGraphics, "Initialized with %s pixel format\n", SDL_GetPixelFormatName(PixelFormat.SDLPixelFormatEnum));
}

void VGraphics::ShutDown()
{
    // Shut down TTF
    {
        TTF_CloseFont(Font);
        TTF_Quit();
    }

    // Destroy surfaces
    {
        BackSurface->Destroy();
        delete VideoSurface;
        delete BackSurface;
    }
}

void VGraphics::PrepareToRender()
{
    FillRect(BackSurface, nullptr, _RGB32(0XFF, 0x00, 0x00));
}

void VGraphics::Render()
{
    Flip();
}

void VGraphics::Flip()
{
    BlitSurface(BackSurface, VideoSurface, nullptr, nullptr);
    SDL_UpdateWindowSurface(Window.SDLWindow);
}

void VGraphics::DrawText(const char* Text, i32 X, i32 Y, u32 Color)
{
    
}
