#pragma once

#include "SDL.h"
#include "Engine/Core/Types.h"
#include "Engine/Core/Platform.h"
#include "Engine/Core/Assert.h"
#include "Engine/Math/Rect.h"

class VSurface
{
protected:
    SDL_Surface* SDLSurface;

    u32* Buffer;
    i32 Pitch; // In pixels
    b32 bLocked;

    i32 Width;
    i32 Height;

public:
    void Create(i32 InWidth, i32 InHeight);
    void Create(SDL_Surface* InSDLSurface);

    void Load(const char* Path);
    void Destroy();

    void Lock(u32*& OutBuffer, i32& OutPitch);
    void Unlock();

    VL_FINLINE u32* GetBuffer()
    {
        VL_ASSERT(bLocked);
        return Buffer;
    }
    VL_FINLINE i32 GetPitch() const
    {
        VL_ASSERT(bLocked);
        return Pitch;
    }
    VL_FINLINE b32 IsLocked() const
    {
        return bLocked;
    }

    VL_FINLINE i32 GetWidth() const 
    {
        return Width;
    }
    VL_FINLINE i32 GetHeight() const
    {
        return Height;
    }
    void BlitHW(VRelRectI* SourceRect, VSurface* Dest, VRelRectI* DestRect);
    void FillRectHW(VRelRectI* Rect, u32 Color);

    void DrawBlended(const VRelRectI* SrcRect, VSurface* Dest, const VRelRectI* DestRect);

    friend class VRenderer;
};