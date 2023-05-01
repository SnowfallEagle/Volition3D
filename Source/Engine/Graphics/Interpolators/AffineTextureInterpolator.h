#pragma once

#include "Engine/Graphics/Interpolators/IInterpolator.h"
#include "Common/Math/Fixed16.h"

namespace Volition
{

class VAffineTextureInterpolator final : public IInterpolator
{
private:
    fx16 UVtx[3], VVtx[3];

    fx16 U, V;

    fx16 ULeft, VLeft;
    fx16 URight, VRight;

    fx16 UDeltaLeftByY, VDeltaLeftByY;
    fx16 UDeltaRightByY, VDeltaRightByY;

    fx16 UDeltaByX, VDeltaByX;

    const u32* TextureBuffer;
    i32 TexturePitch;

public:
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
