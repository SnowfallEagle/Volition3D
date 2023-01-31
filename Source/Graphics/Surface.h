#pragma once

#include "SDL.h"
#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/Assert.h"
#include "Math/Rect.h"

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

    FINLINE u32* GetBuffer()
    {
        ASSERT(bLocked);
        return Buffer;
    }
    FINLINE i32 GetPitch() const
    {
        ASSERT(bLocked);
        return Pitch;
    }
    FINLINE b32 IsLocked() const
    {
        return bLocked;
    }

    FINLINE i32 GetWidth() const 
    {
        return Width;
    }
    FINLINE i32 GetHeight() const
    {
        return Height;
    }
    void BlitHW(VRelRectI* SourceRect, VSurface* Dest, VRelRectI* DestRect);
    void FillRectHW(VRelRectI* Rect, u32 Color);

    void DrawBlended(const VRelRectI* SrcRect, VSurface* Dest, const VRelRectI* DestRect);

    friend class VRenderer;
};
