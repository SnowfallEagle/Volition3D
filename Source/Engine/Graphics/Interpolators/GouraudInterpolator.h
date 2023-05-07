#pragma once

#include "Engine/Graphics/Interpolators/IInterpolator.h"

namespace Volition
{

class VGouraudInterpolator : public IInterpolator
{
public:
    fx16 RVtx[3], GVtx[3], BVtx[3];

    fx16 R, G, B;

    fx16 RLeft, GLeft, BLeft;
    fx16 RRight, GRight, BRight;

    fx16 RDeltaLeftByY, GDeltaLeftByY, BDeltaLeftByY;
    fx16 RDeltaRightByY, GDeltaRightByY, BDeltaRightByY;

    fx16 RDeltaByX, GDeltaByX, BDeltaByX;

public:
    VGouraudInterpolator();
};

}
