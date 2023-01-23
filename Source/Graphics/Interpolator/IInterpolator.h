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

    virtual void Start(const VPolyFace& Poly, const i32 VtxIndices[3]) = 0;

    virtual void ComputeYStartsAndDeltasLeft(i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx) = 0;
    virtual void ComputeYStartsAndDeltasRight(i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx) = 0;
    // TODO(sean): Implement here
    virtual void ComputeYStartsAndDeltas(i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx, i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx) = 0;
    virtual void InterpolateYOverClip(i32 YOverClipLeft, i32 YOverClipRight) = 0;

    virtual void SwapLeftRight() = 0;

    virtual void ComputeXStartsAndDeltas(i32 XDiff) = 0;
    virtual void InterpolateXOverClip(i32 XOverClip) = 0;

    virtual VColorARGB ComputePixel() = 0;

    virtual void InterpolateX() = 0;
    virtual void InterpolateYLeft() = 0;
    virtual void InterpolateYRight() = 0;
    // TODO(sean): Implement here
    virtual void InterpolateY() = 0;
};