#pragma once

#include "Engine/Core/Types/Common.h"
#include "Engine/Core/Platform/Platform.h"
#include "Engine/Core/Config/WindowSpecification.h"

struct SDL_Window;

namespace Volition
{

class VWindow
{
    SDL_Window* SDLWindow;

public:
    void Create();
    void Destroy();
    void HandleEvents();

    friend class VRenderer;
};

extern VWindow Window;

}