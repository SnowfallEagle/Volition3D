#pragma once

#include "Common/Platform/Platform.h"
#include "Common/Types/Common.h"

namespace Volition
{

using fx28 = i32;

static constexpr i32f Fx28Shift = 28;

VLN_FINLINE fx28 IntToFx28(i32 Int)
{
    return Int << Fx28Shift;
}

VLN_FINLINE fx28 Fx28ToInt(fx28 Fixed)
{
    return Fixed >> Fx28Shift;
}

}