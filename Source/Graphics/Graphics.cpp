#include "Graphics/Graphics.h"

VPixelFormat PixelFormat;
VGraphics Graphics;

void VGraphics::StartUp()
{
    SDL_Surface* SDLSurface = SDL_GetWindowSurface(Window.GetWindow());
    ASSERT(SDLSurface);

    // Pixel format
    {
        SDL_PixelFormat* SDLFormat = SDLSurface->format;
        PixelFormat = {
            SDLFormat->Aloss, SDLFormat->Rloss, SDLFormat->Gloss, SDLFormat->Bloss,
            SDLFormat->Ashift, SDLFormat->Rshift, SDLFormat->Gshift, SDLFormat->Bshift,
            SDLFormat->Amask, SDLFormat->Rmask, SDLFormat->Gmask, SDLFormat->Bmask,
            SDLFormat->BytesPerPixel, SDLFormat->BitsPerPixel
        };
    }

    // Video surface
    {
        VideoSurface = VSurface::Create(SDLSurface);
        VideoBuffer = VideoSurface->GetBuffer();
        VideoPitch = VideoSurface->GetPitch();
    }

    // Back surface
    {
        SDL_Surface* Temp = SDL_CreateRGBSurfaceWithFormat(
            0, VideoSurface->GetWidth(), VideoSurface->GetHeight(),
            PixelFormat.BitsPerPixel, VideoSurface->GetPlatformSurface()->format->format
        );
        ASSERT(Temp);

        SDL_Surface* Converted = SDL_ConvertSurface(
            Temp, VideoSurface->GetPlatformSurface()->format, 0
        );
        ASSERT(Converted);
        SDL_FreeSurface(Temp);

        BackSurface = VSurface::Create(Converted);
        BackBuffer = BackSurface->GetBuffer();
        BackPitch = BackSurface->GetPitch();
    }
}
void VGraphics::ShutDown()
{
}

void VGraphics::PrepareToRender()
{
    FillRect(BackSurface, nullptr, _RGB32(0XFF, 0x00, 0x00));
}

void VGraphics::Render()
{
    /*
    i32f Height = BackSurface.GetHeight();
    i32f Width = BackSurface.GetWidth();
    u32* Buffer = BackBuffer;
    for (i32f Y = 0; Y < Height; ++Y)
    {
        for (i32f X = 0; X < Width; ++X)
        {
            Buffer[X] = _RGB32(rand() % 256, rand() % 256, rand() % 256);
        }
        Buffer += BackPitchInPixels;
    }
    */

    VSurface* Surface = VSurface::Load("Test.bmp");
    Surface->EnableColorKey();

    Blit(Surface, nullptr, BackSurface, nullptr);

    Surface->Destroy();
    delete Surface;

    Flip();
}

void VGraphics::Flip()
{
    /* It's emulated blitting
    i32f Height = VideoSurface.GetHeight();
    i32f BytesWidth = VideoSurface.GetWidth() * PixelFormat.BytesPerPixel;
    u32* TempVideo = VideoBuffer;
    u32* TempBack = BackBuffer;

    for (i32f Y = 0; Y < Height; ++Y)
    {
        memcpy(TempVideo, TempBack, BytesWidth);
        TempVideo += VideoPitch;
        TempBack += BackPitch;
    }
    */

    Blit(BackSurface, nullptr, VideoSurface, nullptr);
    SDL_UpdateWindowSurface(Window.GetWindow());
}
