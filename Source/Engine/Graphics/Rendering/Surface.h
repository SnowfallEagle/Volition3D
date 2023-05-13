#pragma once

#include "SDL.h"
#include "Common/Types/Common.h"
#include "Common/Math/Rect.h"
#include "Common/Platform/Platform.h"
#include "Common/Platform/Assert.h"

namespace Volition
{

class VSurface
{
protected:
    SDL_Surface* SDLSurface = nullptr;

    u32* Buffer = nullptr;
    i32 Pitch   = 0; /** In pixels */
    b32 bLocked = false;

    i32 Width  = 0;
    i32 Height = 0;

public:
    void Create(i32 InWidth, i32 InHeight);
    void Create(SDL_Surface* InSDLSurface);

    void Load(const char* Path);
    void Destroy();

    void Lock(u32*& OutBuffer, i32& OutPitch);
    void Unlock();

    void CorrectColors(const VVector3& ColorCorrection = { 1.0f, 1.0f, 1.0f });

    VLN_FINLINE u32* GetBuffer()
    {
        VLN_ASSERT(bLocked);
        return Buffer;
    }

    VLN_FINLINE const u32* GetBuffer() const
    {
        VLN_ASSERT(bLocked);
        return Buffer;
    }

    VLN_FINLINE i32 GetPitch() const
    {
        VLN_ASSERT(bLocked);
        return Pitch;
    }

    VLN_FINLINE b32 IsLocked() const
    {
        return bLocked;
    }

    VLN_FINLINE i32 GetWidth() const 
    {
        return Width;
    }

    VLN_FINLINE i32 GetHeight() const
    {
        return Height;
    }

    VLN_FINLINE void BlitHW(VRelativeRectInt* SourceRect, VSurface* Dest, VRelativeRectInt* DestRect)
    {
        SDL_BlitScaled(SDLSurface, (SDL_Rect*)SourceRect, Dest->SDLSurface, (SDL_Rect*)DestRect);
    }

    VLN_FINLINE void FillRectHW(VRelativeRectInt* Rect, u32 Color)
    {
        SDL_FillRect(SDLSurface, (SDL_Rect*)Rect, Color); // SDL_Rect has the same footprint as VRelativeRectInt
    }

    friend class VRenderer;
};

VLN_FINLINE void VSurface::Lock(u32*& OutBuffer, i32& OutPitch)
{
    // We don't have to lock SDL surface since we don't use RLE
    VLN_ASSERT(!bLocked);

    OutBuffer = Buffer = (u32*)SDLSurface->pixels;
    OutPitch = Pitch = SDLSurface->pitch >> 2; // Divide by 4 (Bytes per pixel)

    bLocked = true;
}

VLN_FINLINE void VSurface::Unlock()
{
    VLN_ASSERT(bLocked);
    bLocked = false;
}

}
