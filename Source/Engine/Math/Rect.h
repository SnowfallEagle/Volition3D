#pragma once

#include "Engine/Core/Types/Common.h"

namespace Volition
{

template<class T>
class TRect
{
public:
    T X1, Y1, X2, Y2;
};

using VRectInt = TRect<i32>;
using VRect = TRect<f32>;

template<class T>
class TRelativeRect
{
public:
    T X, Y, W, H;
};

using VRelativeRectInt = TRelativeRect<i32>;
using VRelativeRect = TRelativeRect<f32>;

}
