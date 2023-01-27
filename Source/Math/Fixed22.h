#pragma once

#include "Core/Platform.h"
#include "Core/Types.h"

typedef i32 fx22;

static constexpr i32f Fx22Shift = 22;

FINLINE fx22 IntToFx22(i32 Int)
{
    return Int << Fx22Shift;
}

FINLINE fx22 Fx22ToInt(fx22 Fixed)
{
    return Fixed >> Fx22Shift;
}
