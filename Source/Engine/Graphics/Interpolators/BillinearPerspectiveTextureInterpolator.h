#pragma once

#include "Engine/Graphics/Interpolators/IInterpolator.h"

namespace Volition
{

class VBillinearPerspectiveTextureInterpolator final : public IInterpolator
{
private:
    fx22 UVtx[3], VVtx[3];

    fx22 U, V;

    fx22 ULeft, VLeft;
    fx22 URight, VRight;

    fx22 UDeltaLeftByY, VDeltaLeftByY;
    fx22 UDeltaRightByY, VDeltaRightByY;

    fx22 UDeltaByX, VDeltaByX;

    const u32* TextureBuffer;
    i32 TexturePitch;
    i32 TextureSize;

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
