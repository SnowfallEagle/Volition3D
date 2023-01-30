#include "Core/Platform.h"

#if VL_IMPL_SDL

#include "Core/Volition.h"
#include "Core/Assert.h"
#include "Core/Impl/Window_SDL.h"

void VSDLWindow::Create(const VWindowSpecification& WindowSpec)
{
    // Init SDL
    {
        i32 Res = SDL_Init(SDL_INIT_VIDEO);
        ASSERT(Res == 0);
    }

    // Set flags
    u32 Flags = SDL_WINDOW_SHOWN;
    {
        if (WindowSpec.Flags & EWindowSpecificationFlags::Fullscreen)
        {
            Flags |= SDL_WINDOW_FULLSCREEN;
        }
        else if (WindowSpec.Flags & EWindowSpecificationFlags::FullscreenDesktop)
        {
            Flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }

        if (WindowSpec.Flags & EWindowSpecificationFlags::Resizable)
        {
            // TODO(sean): Add support for resizing window
            Flags |= SDL_WINDOW_RESIZABLE;
        }
    }

    // Create window
    {
        SDLWindow = SDL_CreateWindow(
            WindowSpec.Name,
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            WindowSpec.Size.X, WindowSpec.Size.Y,
            Flags
        );
        ASSERT(SDLWindow);
    }
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
