#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Core/Platform.h"
#include "Engine/Core/WindowSpecification.h"

struct SDL_Window;

namespace Volition
{

class VWindow
{
    SDL_Window* SDLWindow;

public:
    void Create(const VWindowSpecification& WindowSpec);
    void Destroy();
    void HandleEvents();

    friend class VRenderer;
};

extern VWindow Window;

}