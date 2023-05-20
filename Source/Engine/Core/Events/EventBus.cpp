#include "SDL.h"
#include "Engine/Core/DebugLog.h"
#include "Engine/Core/Events/EventBus.h"

namespace Volition
{

void VEventBus::StartUp()
{
    Events.Resize(InitialEventsCapacity);
}

void VEventBus::Update()
{
    Events.Clear();

    VEvent Event;
    SDL_Event SDLEvent;

    while (SDL_PollEvent(&SDLEvent))
    {
        switch (SDLEvent.type)
        {
        case SDL_QUIT:
        {
            Event.Id = EEventId::Quit;
            PushEvent(Event);
        } break;

        case SDL_WINDOWEVENT:
        {
            if (SDLEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                Event.Id = EEventId::WindowSizeChanged;
                Event.WindowSizeChanged.Width = SDLEvent.window.data1;
                Event.WindowSizeChanged.Height = SDLEvent.window.data2;

                PushEvent(Event);
            }
        } break;

        case SDL_KEYDOWN:
        {
            Event.Id = EEventId::KeyDown;
            Event.KeyDown.Key = SDLEvent.key.keysym.sym;

            PushEvent(Event);
        } break;

        case SDL_MOUSEMOTION:
        {
            Event.Id = EEventId::MouseMove;

            Event.MouseMove.XAbsolute = SDLEvent.motion.x;
            Event.MouseMove.YAbsolute = SDLEvent.motion.y;

            Event.MouseMove.XRelative = SDLEvent.motion.xrel;
            Event.MouseMove.YRelative = SDLEvent.motion.yrel;

            PushEvent(Event);
        } break;

        default: {} break;
        }
    }
}

void VEventBus::PushEvent(const VEvent& Event)
{
    Events.EmplaceBack(Event);
}

const VEvent* VEventBus::GetEventById(EEventId Id) const
{
    for (const auto& Event : Events)
    {
        if (Event.Id == Id)
        {
            return &Event;
        }
    }

    return nullptr;
}

void VEventBus::GetEventsById(TArray<const VEvent*>& Result, EEventId Id) const
{
    for (const auto& Event : Events)
    {
        if (Event.Id == Id)
        {
            Result.EmplaceBack(&Event);
        }
    }
}

}