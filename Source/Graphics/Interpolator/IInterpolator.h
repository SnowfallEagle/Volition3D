/* TODO:
    If virtual methods'll be slow, we'll use function pointers
*/

#pragma once

#include "Graphics/Polygon.h"
#include "Graphics/Color.h"

class IInterpolator
{
public:
    virtual ~IInterpolator() = default;

    virtual void Start(const VPolyFace& Poly, const i32 VtxIndices[3]) {}

    virtual void ComputeYStartsAndDeltasLeft(i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx) {}
    virtual void ComputeYStartsAndDeltasRight(i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx) {}
    FINLINE void ComputeYStartsAndDeltas(i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx, i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx)
    {
        ComputeYStartsAndDeltasLeft(YDiffLeft, LeftStartVtx, LeftEndVtx);
        ComputeYStartsAndDeltasRight(YDiffRight, RightStartVtx, RightEndVtx);
    }

    virtual void ComputeXStartsAndDeltas(i32 XDiff) {}

    virtual void SwapLeftRight() {}

    virtual VColorARGB ComputePixel() { return 0; }

    virtual void InterpolateX(i32 X = 1) {}

    virtual void InterpolateYLeft(i32 YLeft = 1) {}
    virtual void InterpolateYRight(i32 YRight = 1) {}
    FINLINE void InterpolateY(i32 YLeft = 1, i32 YRight = 1)
    {
        InterpolateYLeft(YLeft);
        InterpolateYRight(YRight);
    }

    virtual void End() {}
};