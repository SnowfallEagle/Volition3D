#pragma once

#include "Engine/Graphics/Interpolators/IInterpolator.h"

namespace Volition
{

class VGouraudInterpolator final : public IInterpolator
{
private:
    fx16 RVtx[3], GVtx[3], BVtx[3];

    fx16 R, G, B;

    fx16 RLeft, GLeft, BLeft;
    fx16 RRight, GRight, BRight;

    fx16 RDeltaLeftByY, GDeltaLeftByY, BDeltaLeftByY;
    fx16 RDeltaRightByY, GDeltaRightByY, BDeltaRightByY;

    fx16 RDeltaByX, GDeltaByX, BDeltaByX;

public:
    virtual ~VGouraudInterpolator() = default;

    virtual void Start() override;

    virtual void ComputeYStartsAndDeltasLeft(i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx) override;
    virtual void ComputeYStartsAndDeltasRight(i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx) override;

    virtual void SwapLeftRight() override;
    virtual void ComputeXStartsAndDeltas(i32 XDiff, fx28 ZLeft, fx28 ZRight) override;

    virtual void ProcessPixel() override;
    virtual void InterpolateX(i32 X) override;

    virtual void InterpolateYLeft(i32 YLeft) override;
    virtual void InterpolateYRight(i32 YRight) override;
};

}
