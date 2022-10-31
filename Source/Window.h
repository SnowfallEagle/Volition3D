#ifndef WINDOW_H_

#include "SDL.h"
#include "Types.h"
#include "Assert.h"

namespace EWindowFlag
{
    enum
    {
        Fullscreen = 1 << 0,
    };
}

class VWindow
{
    SDL_Window* SDLWindow;

public:
    void Create(const char* Title, i32 Width, i32 Height, u32 Flags = 0u);
    void Destroy();
    void HandleEvents();
};

extern VWindow Window;

#define WINDOW_H_
#endif
