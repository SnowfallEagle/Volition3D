#include "SDL.h"
#include "Core/Platform.h"

#if VL_IMPL_SDL

#include "Core/Assert.h"
#include "Graphics/Renderer.h"
#include "Graphics/Impl/Surface_SDL.h"

void VSDLSurface::Create(SDL_Surface* InSDLSurface)
{
    ASSERT(InSDLSurface);
    Destroy();

    SDLSurface = InSDLSurface;
    Width = InSDLSurface->w;
    Height = InSDLSurface->h;
}

void VSDLSurface::Create(i32 InWidth, i32 InHeight)
{
    Destroy();

    SDL_Surface* PlatformSurface = SDL_CreateRGBSurfaceWithFormat(
        0, InWidth, InHeight, Renderer.BitsPerPixel,
        Renderer.SDLPixelFormatEnum
    );
    ASSERT(PlatformSurface);

    SDLSurface = PlatformSurface;
    Width = SDLSurface->w;
    Height = SDLSurface->h;
}

void VSDLSurface::Load(const char* Path)
{
    Destroy();

    SDL_Surface* Temp = SDL_LoadBMP(Path);
    ASSERT(Temp);

    SDL_Surface* Converted = SDL_ConvertSurface(
        Temp, Renderer.SDLPixelFormat, 0
    );
    ASSERT(Converted);
    SDL_FreeSurface(Temp);

    Create(Converted);
}

void VSDLSurface::Destroy()
{
    if (SDLSurface)
    {
        SDL_FreeSurface(SDLSurface);
        SDLSurface = nullptr;
    }

    Buffer = nullptr;
    Pitch = 0;
    bLocked = false;
    Width = 0;
    Height = 0;
}

void VSDLSurface::Lock(u32*& OutBuffer, i32& OutPitch)
{
    ASSERT(!bLocked);

    /* NOTE(sean):
        We don't have to lock SDL surface
        since we don't use RLE
     */

    OutBuffer = Buffer = (u32*)SDLSurface->pixels;
    OutPitch = Pitch = SDLSurface->pitch >> 2; // Divide by 4 (Bytes per pixel)

    bLocked = true;
}

void VSDLSurface::Unlock()
{
    ASSERT(bLocked);

    bLocked = false;
}

void VSDLSurface::FillRectHW(VRelRectI* Rect, u32 Color)
{
    // SDL_Rect has the same footprint as VRelRectI
    SDL_FillRect(SDLSurface, (SDL_Rect*)Rect, Color);
}

void VSDLSurface::BlitHW(VRelRectI* SourceRect, VSDLSurface* Dest, VRelRectI* DestRect)
{
    SDL_BlitScaled(SDLSurface, (SDL_Rect*)SourceRect, Dest->SDLSurface, (SDL_Rect*)DestRect);
}

#endif VL_IMPL_SDL
