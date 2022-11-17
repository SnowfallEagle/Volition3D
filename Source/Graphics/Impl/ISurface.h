#pragma once

#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/Assert.h"

/** NOTE(sean):
    We don't use virtual functions
    for interface because we need speed
 */
class ISurface
{
protected:
    u32* Buffer = nullptr;
    i32 Pitch = 0; // In pixels
    i32 Width = 0;
    i32 Height = 0;
    b32 bLocked = false;

public:
    FINLINE u32* GetBuffer()
    {
        ASSERT(bLocked);
        return Buffer;
    }
    FINLINE i32 GetPitch()
    {
        ASSERT(bLocked);
        return Pitch;
    }
    FINLINE i32 GetWidth() const 
    {
        return Width;
    }
    FINLINE i32 GetHeight() const
    {
        return Height;
    }

protected:
    ISurface() = default;
    ISurface(i32 InWidth, i32 InHeight)
        : Width(InWidth), Height(InHeight)
    {
    }
};
