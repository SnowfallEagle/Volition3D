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

using VRectI = TRect<i32>;
using VRectF = TRect<f32>;

template<class T>
class TRelRect // Relative rectangle
{
public:
    T X, Y, W, H;
};

using VRelRectI = TRelRect<i32>;
using VRelRectF = TRelRect<f32>;

}
