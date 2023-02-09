#pragma once

#include "Engine/Core/Types.h"

template<class T>
class TRect
{
public:
    T X1, Y1, X2, Y2;
};

typedef TRect<i32> VRectI;
typedef TRect<f32> VRectF;

template<class T>
class TRelRect // Relative rectangle
{
public:
    T X, Y, W, H;
};

typedef TRelRect<i32> VRelRectI;
typedef TRelRect<f32> VRelRectF;
