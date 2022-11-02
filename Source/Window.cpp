#include "Volition.h"
#include "Core/Assert.h"
#include "Window.h"

VWindow Window;

void VWindow::Create(const char* Title, i32 Width, i32 Height, u32 Flags)
{
    // Init SDL
    i32 Res = SDL_Init(SDL_INIT_VIDEO);
    ASSERT(Res == 0);

    // Set up window flags
    Uint32 SDLFlags = 0;
    if (Flags & EWindowFlag::Fullscreen)
        SDLFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

    // Create window
    SDLWindow = SDL_CreateWindow(
        Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        Width, Height, Flags
    );
    ASSERT(SDLWindow);
}

void VWindow::Destroy()
{
    SDL_DestroyWindow(SDLWindow);
    SDL_Quit();
}

void VWindow::HandleEvents()
{
    SDL_Event Event;

    while (SDL_PollEvent(&Event))
    {
        switch (Event.type)
        {
        case SDL_QUIT:
        {
            Volition.Stop();
        } break;

        default: {} break;
        }
    }
}
