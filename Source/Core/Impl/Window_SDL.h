#pragma once

#include "SDL.h"
#include "Core/Types.h"

class VSDLWindow
{
    SDL_Window* SDLWindow;

public:
    void Create(const VWindowSpecification& WindowSpec);
    void Destroy();
    void HandleEvents();

    friend class VSDLRenderer;
};

typedef VSDLWindow VWindow;
