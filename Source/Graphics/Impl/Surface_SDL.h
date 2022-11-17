#pragma once

#include "SDL.h"
#include "Graphics/Impl/ISurface.h"

class VSurface final : public ISurface
{
    SDL_Surface* SDLSurface = nullptr;

public:
    void Create(SDL_Surface* InSDLSurface);
    virtual void Create(i32 InWidth, i32 InHeight) override;
    virtual void Load(const char* Path) override;
    virtual void Destroy() override;

    virtual void Lock(u32*& OutBuffer, i32& OutPitch) override;
    virtual void Unlock() override;

    friend class VRenderer;
};
