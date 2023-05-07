#pragma once

#include "Engine/Graphics/Interpolators/IInterpolator.h"
#include "Common/Math/Fixed22.h"

namespace Volition
{

class VLinearPiecewiseTextureInterpolator : public IInterpolator
{
public:
    fx22 UVtx[3], VVtx[3];

    fx22 U, V;

    fx22 ULeft, VLeft;
    fx22 URight, VRight;

    fx22 UDeltaLeftByY, VDeltaLeftByY;
    fx22 UDeltaRightByY, VDeltaRightByY;

    fx22 UDeltaByX, VDeltaByX;

    const u32* TextureBuffer;
    i32 TexturePitch;

public:
    VLinearPiecewiseTextureInterpolator();
};

}
