#ifndef CORE_WINDOW_H_

#include "SDL.h"
#include "Core/Types.h"
#include "Core/Platform.h"

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

    friend class VGraphics;
};

extern VWindow Window;

#define CORE_WINDOW_H_
#endif
