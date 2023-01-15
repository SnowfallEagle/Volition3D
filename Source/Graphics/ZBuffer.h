#pragma once

#include "Core/Memory.h"
#include "Math/Fixed16.h"
#include "Graphics/Surface.h"

class VZBuffer
{
private:
    VSurface Surface;
public:
    u32* Buffer;
    i32 Pitch;
    i32 Width;
    i32 Height;

public:
    void Create(i32 InWidth, i32 InHeight)
    {
        Surface.Create(InWidth, InHeight);
        Surface.Lock(Buffer, Pitch);
        Width = InWidth;
        Height = InHeight;
    }

    FINLINE void Destroy()
    {
        Surface.Unlock();
        Surface.Destroy();
    }

    FINLINE void Clear()
    {
        static constexpr i32 ZBufferClearValue = 16000;
        Memory.MemSetQuad(Buffer, IntToFx16(ZBufferClearValue), Pitch * Height);
    }
};
