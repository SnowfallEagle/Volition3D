#pragma once

#include "Engine/Graphics/Polygon.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Math/Fixed16.h"
#include "Engine/Math/Fixed22.h"
#include "Engine/Math/Fixed28.h"

class IInterpolator
{
public:
    virtual ~IInterpolator() = default;

    virtual void Start(const u32* Buffer, i32 Pitch, const VPolyFace& Poly, const i32 VtxIndices[3]) {}

    virtual void ComputeYStartsAndDeltasLeft(i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx) {}
    virtual void ComputeYStartsAndDeltasRight(i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx) {}
    VL_FINLINE void ComputeYStartsAndDeltas(i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx, i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx)
    {
        ComputeYStartsAndDeltasLeft(YDiffLeft, LeftStartVtx, LeftEndVtx);
        ComputeYStartsAndDeltasRight(YDiffRight, RightStartVtx, RightEndVtx);
    }

    virtual void ComputeXStartsAndDeltas(i32 XDiff, fx28 ZLeft, fx28 ZRight) {}

    virtual void SwapLeftRight() {}

    virtual VColorARGB ProcessPixel(VColorARGB Pixel, i32f X, i32f Y, fx28 Z) { return Pixel; }

    virtual void InterpolateX(i32 X = 1) {}

    virtual void InterpolateYLeft(i32 YLeft = 1) {}
    virtual void InterpolateYRight(i32 YRight = 1) {}
    VL_FINLINE void InterpolateY(i32 YLeft = 1, i32 YRight = 1)
    {
        InterpolateYLeft(YLeft);
        InterpolateYRight(YRight);
    }

    virtual void End() {}
};