#pragma once

#include "Common/Platform/Memory.h"
#include "Common/Math/Fixed28.h"
#include "Engine/Graphics/Rendering/Surface.h"

namespace Volition
{

class VZBuffer
{
public:
    u32* Buffer;
    i32 Pitch;
    i32 Width;
    i32 Height;

    b32 bInitialized = false;

private:
    VSurface Surface;

public:
    void Create(i32 InWidth, i32 InHeight)
    {
        Destroy();

        Surface.Create(InWidth, InHeight);
        Surface.Lock(Buffer, Pitch);
        Width = InWidth;
        Height = InHeight;

        bInitialized = true;
    }

    void Destroy()
    {
        if (bInitialized)
        {
            Surface.Unlock();
            Surface.Destroy();

            bInitialized = false;
        }
    }

    VLN_FINLINE void Clear()
    {
        Memory.MemSetQuad(Buffer, 0, Pitch * Height);
    }
};

}
