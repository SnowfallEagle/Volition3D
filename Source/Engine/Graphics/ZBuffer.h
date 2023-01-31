#pragma once

#include "Engine/Core/Memory.h"
#include "Engine/Math/Fixed28.h"
#include "Engine/Graphics/Surface.h"

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

    VL_FINLINE void Destroy()
    {
        Surface.Unlock();
        Surface.Destroy();
    }

    VL_FINLINE void Clear()
    {
        Memory.MemSetQuad(Buffer, 0, Pitch * Height);
    }
};
