#include <string.h>
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

    static constexpr i32f CharsPerLine = 80;
    static constexpr f32 PointDivPixel = 0.75f;
    static constexpr f32 QualityMultiplier = 2.0f;
    FontCharWidth = ScreenWidth / CharsPerLine;
    FontCharHeight = (i32)(FontCharWidth * 1.25f);

    Font = TTF_OpenFont("Default.ttf", (i32f)( (f32)FontCharWidth * PointDivPixel * QualityMultiplier ));
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
    // Convert our color
    SDL_Color SDLColor;
    SDLColor.a = Color & _ALPHA_MASK;
    SDLColor.r = Color & _RED_MASK;
    SDLColor.g = Color & _GREEN_MASK;
    SDLColor.b = Color & _BLUE_MASK;

    // Render text
    SDL_Surface* SDLSurface = TTF_RenderText_Solid(Font, Text, SDLColor);
    ASSERT(SDLSurface);
    //SDL_SetColorKey(SDLSurface, SDL_TRUE, static_cast<u32*>(SDLSurface->pixels)[0]);

    // Convert surface
    SDL_Surface* SDLConverted = SDL_ConvertSurface(SDLSurface, PixelFormat.SDLPixelFormat, 0);
    ASSERT(SDLConverted);

    // Blit
    SDL_Rect Dest = {
        X, Y, (i32f)strlen(Text) * FontCharWidth, FontCharHeight
    };
    SDL_BlitScaled(SDLConverted, nullptr, BackSurface->SDLSurface, &Dest);

    // Free memory
    SDL_FreeSurface(SDLSurface);
    SDL_FreeSurface(SDLConverted);
}
