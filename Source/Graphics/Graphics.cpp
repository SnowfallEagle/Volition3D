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
    Flip();
}

void VGraphics::Flip()
{
    BlitSurface(BackSurface, nullptr, VideoSurface, nullptr);
    SDL_UpdateWindowSurface(Window.GetWindow());
}
