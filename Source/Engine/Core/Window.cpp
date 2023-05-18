#include "SDL.h"
#include "Common/Platform/Assert.h"
#include "Engine/Core/Engine.h"
#include "Engine/Core/Config/Config.h"
#include "Engine/Core/Events/EventBus.h"
#include "Engine/Core/Window.h"

namespace Volition
{

void VWindow::StartUp()
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
        else if (Config.WindowSpec.Flags & EWindowSpecificationFlags::Borderless)
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

void VWindow::ShutDown()
{
    SDL_DestroyWindow(SDLWindow);
    SDL_Quit();
}

void VWindow::ProcessEvents()
{
    if (EventBus.GetEventById(EEventId::Quit))
    {
        Engine.Stop();
    }

    if (EventBus.GetEventById(EEventId::WindowSizeChanged))
    {
        Renderer.RefreshWindowSurface();
    }
}

}
