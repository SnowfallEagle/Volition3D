#ifndef CORE_IMPL_WINDOW_SDL_H_

#include "SDL.h"
#include "Core/Types.h"

class VWindow
{
    SDL_Window* SDLWindow;

public:
    void Create(const char* Title, i32 Width, i32 Height);
    void Destroy();
    void HandleEvents();

    friend class VRenderer;
};

#define CORE_IMPL_WINDOW_SDL_H_
#endif