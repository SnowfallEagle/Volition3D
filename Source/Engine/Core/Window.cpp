#include "SDL.h"
#include "Engine/Core/Platform/Assert.h"
#include "Engine/Core/Engine.h"
#include "Engine/Core/Config/Config.h"
#include "Engine/Core/Window.h"

namespace Volition
{

VWindow Window;

void VWindow::Create()
{
    // Init SDL
    {
        const i32 Res = SDL_Init(SDL_INIT_VIDEO);
        VLN_ASSERT(Res == 0);
    }

    // Set flags
    u32 Flags = SDL_WINDOW_SHOWN;
    {
        if (Config.WindowSpec.Flags & EWindowSpecificationFlags::Fullscreen)
        {
            Flags |= SDL_WINDOW_FULLSCREEN;
        }
        else if (Config.WindowSpec.Flags & EWindowSpecificationFlags::FullscreenDesktop)
        {
            Flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
    }

    // Create window
    {
        SDLWindow = SDL_CreateWindow(
            Config.WindowSpec.Name,
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            Config.WindowSpec.Size.X, Config.WindowSpec.Size.Y,
            Flags
        );
        VLN_ASSERT(SDLWindow);
    }
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
            Engine.Stop();
        } break;

        default: {} break;
        }
    }
}

}
