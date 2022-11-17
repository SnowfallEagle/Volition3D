#pragma once

#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/Assert.h"

class ISurface
{
protected:
    u32* Buffer = nullptr;
    i32 Pitch = 0; // In pixels
    b32 bLocked = false;

    i32 Width = 0;
    i32 Height = 0;

public:
    // TODO(sean): Am i idiot? I can make them non-static...

    // Static stuff that can't be virtual:
    // virtual static ISurface* Create(i32 InWidth, i32 InHeight) = 0;
    // virtual static ISurface* Load(const char* Path) = 0;
    virtual void Destroy() = 0;

    virtual void Lock(u32*& OutBuffer, i32& OutPitch) = 0;
    virtual void Unlock() = 0;

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

protected:
    ISurface() = default;
    ISurface(i32 InWidth, i32 InHeight)
        : Width(InWidth), Height(InHeight)
    {
    }
};
