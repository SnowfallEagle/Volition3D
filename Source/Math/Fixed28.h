#pragma once

#include "Core/Platform.h"
#include "Core/Types.h"

typedef i32 fx28;

static constexpr i32f Fx28Shift = 28;

VL_FINLINE fx28 IntToFx28(i32 Int)
{
    return Int << Fx28Shift;
}

VL_FINLINE fx28 Fx28ToInt(fx28 Fixed)
{
    return Fixed >> Fx28Shift;
}