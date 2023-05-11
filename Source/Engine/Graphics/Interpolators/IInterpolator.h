#pragma once

#include "Common/Math/Fixed16.h"
#include "Common/Math/Fixed22.h"
#include "Common/Math/Fixed28.h"
#include "Engine/Graphics/Types/Polygon.h"
#include "Engine/Graphics/Types/Color.h"

namespace Volition
{

class IInterpolator
{
public:
    using StartType = void (*)(IInterpolator*);
    using ComputeYStartsAndDeltasLeftType  = void (*)(IInterpolator*, i32, i32, i32);
    using ComputeYStartsAndDeltasRightType = void (*)(IInterpolator*, i32, i32, i32);
    using ComputeXStartsAndDeltasType      = void (*)(IInterpolator*, i32, fx28, fx28);
    using SwapLeftRightType = void (*)(IInterpolator*);
    using ProcessPixelType  = void (*)(IInterpolator*);
    using InterpolateXType      = void (*)(IInterpolator*, i32);
    using InterpolateYLeftType  = void (*)(IInterpolator*, i32);
    using InterpolateYRightType = void (*)(IInterpolator*, i32);

public:
    class VInterpolationContext* InterpolationContext;

public:
    IInterpolator();
    virtual ~IInterpolator() = default;

    void (*Start)(IInterpolator* Self);

    void (*ComputeYStartsAndDeltasLeft)(IInterpolator* Self, i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx);
    void (*ComputeYStartsAndDeltasRight)(IInterpolator* Self, i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx);

    VLN_FINLINE void ComputeYStartsAndDeltas(i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx, i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx)
    {
        ComputeYStartsAndDeltasLeft(this, YDiffLeft, LeftStartVtx, LeftEndVtx);
        ComputeYStartsAndDeltasRight(this, YDiffRight, RightStartVtx, RightEndVtx);
    }

    void (*ComputeXStartsAndDeltas)(IInterpolator* Self, i32 XDiff, fx28 ZLeft, fx28 ZRight);
    void (*SwapLeftRight)(IInterpolator* Self);
    void (*ProcessPixel)(IInterpolator* Self);

    void (*InterpolateX)(IInterpolator* Self, i32 X);

    void (*InterpolateYLeft)(IInterpolator* Self, i32 YLeft);
    void (*InterpolateYRight)(IInterpolator* Self, i32 YRight);

    VLN_FINLINE void InterpolateY(i32 YLeft, i32 YRight)
    {
        InterpolateYLeft(this, YLeft);
        InterpolateYRight(this, YRight);
    }

    VLN_FINLINE void SetInterpolationContext(VInterpolationContext& InInterpolationContext)
    {
        InterpolationContext = &InInterpolationContext;
    }
};

}