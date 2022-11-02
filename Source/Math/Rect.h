#ifndef MATH_RECT_H_

#include "Core/Types.h"

template<class T>
class TRect
{
public:
    T X1, Y1, X2, Y2;
};

typedef TRect<i32> VRectI;
typedef TRect<f32> VRectF;

// NOTE(sean): In memory it's equivalent of SDL_Rect
template<class T>
class TRelativeRect
{
public:
    T X, Y, W, H;
};

typedef TRelativeRect<i32> VRelativeRectI;
typedef TRelativeRect<f32> VRelativeRectF;

#define MATH_RECT_H_
#endif