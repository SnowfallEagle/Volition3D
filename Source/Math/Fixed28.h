#pragma once

#include "Core/Platform.h"
#include "Core/Types.h"

typedef i32 fx28;

namespace
{
    static constexpr i32f Shift = 28;
};

FINLINE fx28 IntToFx28(i32 Int)
{
    return Int << Shift;
}

FINLINE fx28 Fx28ToInt(fx28 Fixed)
{
    return Fixed >> Shift;
}