#pragma once

#include "Engine/Graphics/Interpolators/IInterpolator.h"

namespace Volition
{

class VBilinearPerspectiveTextureInterpolator : public IInterpolator
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
    VVector2 TextureSize;

public:
    VBilinearPerspectiveTextureInterpolator();
};

}
