#pragma once

#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/Assert.h"

class ISurface
{
protected:
    u32* Buffer;
    i32 Pitch; // In pixels
    b32 bLocked;

    i32 Width;
    i32 Height;

public:
    virtual void Create(i32 InWidth, i32 InHeight) = 0;
    virtual void Load(const char* Path) = 0;
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
};
