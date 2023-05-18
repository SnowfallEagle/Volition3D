#pragma once

#include "Common/Platform/Platform.h"
#include "Common/Types/Common.h"
#include "Common/Types/Array.h"

namespace Volition
{

enum class EEventId : i32
{
    Quit,
    WindowSizeChanged,

    KeyDown,
    MouseMove
};

class VEvent
{
public:
    EEventId Id;

    union
    {
        struct
        {
            i32 Width, Height;
        } WindowSizeChanged;

        struct
        {
            i32 Key;
        } KeyDown;

        struct
        {
            i32 XRelative, YRelative;
            i32 XAbsolute, YAbsolute;
        } MouseMove;
    };
};

}