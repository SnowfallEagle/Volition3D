#include "Graphics/Graphics.h"

VGraphics Graphics;

void VGraphics::StartUp()
{
    // Set up video surface
    SDL_Surface* SDLSurface = SDL_GetWindowSurface(Window.GetWindow());
    ASSERT(SDLSurface);
    VideoSurface.SetPlatformSurface(SDLSurface);
    VideoSurface.ToggleOwn(false);

    // Pixel format
    SDL_PixelFormat* SDLFormat = VideoSurface.GetPlatformSurface()->format;
    PixelFormat = {
        SDLFormat->Aloss, SDLFormat->Rloss, SDLFormat->Gloss, SDLFormat->Bloss,
        SDLFormat->Ashift, SDLFormat->Rshift, SDLFormat->Gshift, SDLFormat->Bshift,
        SDLFormat->Amask, SDLFormat->Rmask, SDLFormat->Gmask, SDLFormat->Bmask,
        SDLFormat->BytesPerPixel, SDLFormat->BitsPerPixel
    };

    // Back surface
    SDL_Surface* Temp = SDL_CreateRGBSurfaceWithFormat(
        0, VideoSurface.GetWidth(), VideoSurface.GetHeight(),
        PixelFormat.BitsPerPixel, VideoSurface.GetPlatformSurface()->format->format
    );
    ASSERT(Temp);

    SDL_Surface* Converted = SDL_ConvertSurface(Temp, VideoSurface.GetPlatformSurface()->format, 0);
    ASSERT(Converted);
    SDL_FreeSurface(Temp);

    BackSurface.SetPlatformSurface(Converted);

    // Other
    BackBuffer = nullptr;
    BackPitchInPixels = 0;
}
void VGraphics::ShutDown()
{
}

void VGraphics::PrepareToRender()
{
    SDL_FillRect(BackSurface.GetPlatformSurface(), nullptr, _RGB32(0xFF, 0x00, 0x00));
    BackSurface.Lock(BackBuffer, BackPitchInPixels); // TODO(sean): Maybe it'll be better to lock only when we need to?
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
    VSurface* Surface = LoadBMP("Test.bmp");
    Surface->EnableColorKey();
    Surface->EnableRLE();
    Blit(Surface, nullptr, &BackSurface, nullptr);
    delete Surface;

    Flip();
}

void VGraphics::Flip()
{
    u32* VideoBuffer;
    i32 Pitch;
    VideoSurface.Lock(VideoBuffer, Pitch);
    {
        i32f Height = VideoSurface.GetHeight();
        i32f BytesWidth = VideoSurface.GetWidth() * PixelFormat.BytesPerPixel;

        for (i32f Y = 0; Y < Height; ++Y)
        {
            memcpy(VideoBuffer, BackBuffer, BytesWidth);
            VideoBuffer += Pitch;
            BackBuffer += BackPitchInPixels;
        }
    }
    VideoSurface.Unlock();
    BackSurface.Unlock();

    SDL_UpdateWindowSurface(Window.GetWindow());
}

VSurface* VGraphics::LoadBMP(const char* Path)
{
    SDL_Surface* Temp = SDL_LoadBMP(Path);
    ASSERT(Temp);

    SDL_Surface* Converted = SDL_ConvertSurface(
        Temp, VideoSurface.GetPlatformSurface()->format, 0
    );
    SDL_FreeSurface(Temp);
    ASSERT(Converted);

    return new VSurface(Converted);
}
