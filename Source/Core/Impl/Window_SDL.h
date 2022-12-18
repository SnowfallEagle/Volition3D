#pragma once

#include "SDL.h"
#include "Core/Types.h"

class VSDLWindow
{
    SDL_Window* SDLWindow;

public:
    void Create(const char* Title, i32 Width, i32 Height);
    void Destroy();
    void HandleEvents();

    friend class VSDLRenderer;
};

typedef VSDLWindow VWindow;
