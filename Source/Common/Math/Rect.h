#pragma once

#include "Common/Types/Common.h"

namespace Volition
{

template<typename T>
class TRect
{
public:
    T X1, Y1, X2, Y2;
};

using VRectInt = TRect<i32>;
using VRect = TRect<f32>;

template<typename T>
class TRelativeRect
{
public:
    T X, Y, W, H;
};

using VRelativeRectInt = TRelativeRect<i32>;
using VRelativeRect = TRelativeRect<f32>;

}
