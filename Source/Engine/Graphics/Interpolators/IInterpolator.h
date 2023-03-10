#pragma once

#include "Engine/Math/Fixed16.h"
#include "Engine/Math/Fixed22.h"
#include "Engine/Math/Fixed28.h"
#include "Engine/Graphics/Polygon.h"
#include "Engine/Graphics/Color.h"

namespace Volition
{

class VInterpolationContext;

class IInterpolator
{
protected:
    VInterpolationContext* InterpolationContext;

public:
    virtual ~IInterpolator() = default;

    void SetInterpolationContext(VInterpolationContext& InInterpolationContext)
    {
        InterpolationContext = &InInterpolationContext;
    }
    virtual void Start()
    {}

    virtual void ComputeYStartsAndDeltasLeft(i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx)
    {}
    virtual void ComputeYStartsAndDeltasRight(i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx)
    {}
    VLN_FINLINE void ComputeYStartsAndDeltas(i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx, i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx)
    {
        ComputeYStartsAndDeltasLeft(YDiffLeft, LeftStartVtx, LeftEndVtx);
        ComputeYStartsAndDeltasRight(YDiffRight, RightStartVtx, RightEndVtx);
    }

    virtual void ComputeXStartsAndDeltas(i32 XDiff, fx28 ZLeft, fx28 ZRight)
    {}

    virtual void SwapLeftRight()
    {}

    virtual void ProcessPixel()
    {}

    virtual void InterpolateX(i32 X = 1)
    {}

    virtual void InterpolateYLeft(i32 YLeft = 1)
    {}
    virtual void InterpolateYRight(i32 YRight = 1)
    {}
    VLN_FINLINE void InterpolateY(i32 YLeft = 1, i32 YRight = 1)
    {
        InterpolateYLeft(YLeft);
        InterpolateYRight(YRight);
    }
};

}