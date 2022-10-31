#ifndef WINDOW_H_

#include "SDL.h"
#include "Types.h"
#include "Assert.h"

class VWindow
{
public:
    enum
    {
        FlagFullscreen = 1 << 0,
    };

private:
    SDL_Window* SDLWindow;

public:
    void Create(const char* Title, i32 Width, i32 Height, u32 Flags = 0u)
    {
        // Init SDL
        ASSERT(SDL_Init(SDL_INIT_VIDEO) == 0);

        // Set up window flags
        Uint32 SDLFlags = 0;
        if (Flags & FlagFullscreen)
            SDLFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

        // Create window
        ASSERT(SDLWindow = SDL_CreateWindow(
            Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            Width, Height, Flags
        ));
    }

    void Destroy()
    {
        SDL_DestroyWindow(SDLWindow);
        SDL_Quit();
    }
};

extern VWindow Window;

#define WINDOW_H_
#endif
