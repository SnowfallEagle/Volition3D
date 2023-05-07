#pragma once

#include "Engine/Graphics/Interpolators/IInterpolator.h"
#include "Common/Math/Fixed16.h"

namespace Volition
{

class VAffineTextureInterpolator : public IInterpolator
{
public:
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
    VAffineTextureInterpolator();
};

}
