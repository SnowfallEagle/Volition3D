#pragma once

#include "SDL.h"
#include "Common/Types/Common.h"
#include "Common/Platform/Platform.h"
#include "Common/Platform/Assert.h"
#include "Common/Math/Rect.h"

namespace Volition
{

class VSurface
{
protected:
    SDL_Surface* SDLSurface = nullptr;

    u32* Buffer = nullptr;
    i32 Pitch   = 0; // In pixels
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

    void BlitHW(VRelativeRectInt* SourceRect, VSurface* Dest, VRelativeRectInt* DestRect);
    void FillRectHW(VRelativeRectInt* Rect, u32 Color);

    void DrawBlended(const VRelativeRectInt* SrcRect, VSurface* Dest, const VRelativeRectInt* DestRect);

    friend class VRenderer;
};

}
