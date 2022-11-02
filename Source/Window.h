#ifndef WINDOW_H_

#include "SDL.h"
#include "Types.h"
#include "Platform.h"

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

    FINLINE SDL_Window* GetWindow()
    {
        return SDLWindow;
    }
};

extern VWindow Window;

#define WINDOW_H_
#endif
