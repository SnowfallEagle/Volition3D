#include "Graphics/Graphics.h"

VPixelFormat PixelFormat;
VGraphics Graphics;

void VGraphics::StartUp()
{
    SDL_Surface* SDLSurface = SDL_GetWindowSurface(Window.SDLWindow);
    ASSERT(SDLSurface);

    SDL_PixelFormat* SDLFormat = SDLSurface->format;
    PixelFormat = {
        SDLFormat->Aloss, SDLFormat->Rloss, SDLFormat->Gloss, SDLFormat->Bloss,
        SDLFormat->Ashift, SDLFormat->Rshift, SDLFormat->Gshift, SDLFormat->Bshift,
        SDLFormat->Amask, SDLFormat->Rmask, SDLFormat->Gmask, SDLFormat->Bmask,
        SDLFormat->BytesPerPixel, SDLFormat->BitsPerPixel
    };

    VideoSurface = VSurface::Create(SDLSurface);
    ScreenWidth = VideoSurface->Width;
    ScreenHeight = VideoSurface->Height;
    BackSurface = VSurface::Create(ScreenWidth, ScreenHeight);
}

void VGraphics::ShutDown()
{
    BackSurface->Destroy();
    delete VideoSurface;
    delete BackSurface;
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
