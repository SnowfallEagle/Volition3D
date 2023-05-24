#include "Common/Platform/Assert.h"
#include "Engine/Core/Events/EventBus.h"
#include "Engine/Input/Input.h"

namespace Volition
{

void VInput::StartUp()
{
    SDL_SetRelativeMouseMode(SDL_TRUE);

    KeyDownEvents.Resize(InitialKeyDownEventsCapacity);
    KeyState = SDL_GetKeyboardState(nullptr);
}

void VInput::ShutDown()
{
}

void VInput::ProcessEvents()
{
    MouseState = SDL_GetMouseState(&MouseAbsolutePosition.X, &MouseAbsolutePosition.Y);
    MouseMoveEvent = EventBus.GetEventById(EEventId::MouseMove);
    MouseRelativePosition = MouseMoveEvent ? VVector2i({ MouseMoveEvent->MouseMove.XRelative, MouseMoveEvent->MouseMove.YRelative }) : VVector2i({ 0, 0 });

    KeyDownEvents.Clear();
    EventBus.GetEventsById(KeyDownEvents, EEventId::KeyDown);
}

b32 VInput::IsKeyDown(EKeycode Key) const
{
    for (const auto* Event : KeyDownEvents)
    {
        VLN_ASSERT(Event);

        if (Event->KeyDown.Key == Key)
        {
            return true;
        }
    }

    return KeyState[SDL_GetScancodeFromKey(Key)];
}

b32 VInput::IsEventKeyDown(EKeycode Key) const
{
    for (const auto* Event : KeyDownEvents)
    {
        VLN_ASSERT(Event);

        if (Event->KeyDown.Key == Key)
        {
            return true;
        }
    }

    return false;
}


}