#pragma once

#include "Engine/Core/Platform.h"
#include "Engine/Core/Types.h"

namespace Volition
{

using fx22 = i32;

static constexpr i32f Fx22Shift = 22;

VLN_FINLINE fx22 IntToFx22(i32 Int)
{
    return Int << Fx22Shift;
}

VLN_FINLINE fx22 Fx22ToInt(fx22 Fixed)
{
    return Fixed >> Fx22Shift;
}

}
