#pragma once

#include "SDL.h"
#include "Graphics/Impl/ISurface.h"

class VSurface final : public ISurface
{
    SDL_Surface* SDLSurface = nullptr;

public:
    static VSurface* Create(SDL_Surface* InSDLSurface);
    static VSurface* Create(i32 InWidth, i32 InHeight);
    static VSurface* Load(const char* Path);
    virtual void Destroy() override;

    virtual void Lock(u32*& OutBuffer, i32& OutPitch) override;
    virtual void Unlock() override;

private:
    VSurface() = default;
    VSurface(SDL_Surface* InSDLSurface, i32 InWidth, i32 InHeight)
        : ISurface(InWidth, InHeight), SDLSurface(InSDLSurface)
    {
    }

    friend class VRenderer;
};
