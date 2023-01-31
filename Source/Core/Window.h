#pragma once

#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/WindowSpecification.h"

struct SDL_Window;

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
