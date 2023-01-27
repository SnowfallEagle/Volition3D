#pragma once

#include "Core/Platform.h"
#include "Core/Types.h"

typedef i32 fx22;

namespace
{
    static constexpr i32f Shift = 22;
};

FINLINE fx22 IntToFx22(i32 Int)
{
    return Int << Shift;
}

FINLINE fx22 Fx22ToInt(fx22 Fixed)
{
    return Fixed >> Shift;
}
