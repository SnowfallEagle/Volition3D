#include "Core/Platform.h"

#if VL_IMPL_SDL

#include "Core/Volition.h"
#include "Core/Assert.h"
#include "Core/Impl/Window_SDL.h"

void VSDLWindow::Create(const char* Title, i32 Width, i32 Height)
{
    // Init SDL
    i32 Res = SDL_Init(SDL_INIT_VIDEO);
    ASSERT(Res == 0);

    // Create window
    SDLWindow = SDL_CreateWindow(
        Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        Width, Height, SDL_WINDOW_SHOWN
    );
    ASSERT(SDLWindow);
}

void VSDLWindow::Destroy()
{
    SDL_DestroyWindow(SDLWindow);
    SDL_Quit();
}

void VSDLWindow::HandleEvents()
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

#endif // VL_IMPL_SDL
