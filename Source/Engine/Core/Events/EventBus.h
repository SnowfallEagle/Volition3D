#pragma once

#include "Engine/Core/Events/Event.h"

namespace Volition
{

class VEventBus
{
    static constexpr i32f InitialEventsCapacity = 16;

private:
    TArray<VEvent> Events;

public:
    void StartUp();
    void ShutDown() {}
    void Update();

    /** nullptr if no event */
    const VEvent* GetEventById(EEventId Id) const;
    void GetEventsById(TArray<const VEvent*>& Result, EEventId Id) const;
    void PushEvent(const VEvent& Event);
};

inline VEventBus EventBus;

}