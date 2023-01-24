#pragma once

#include "Graphics/Interpolator/IInterpolator.h"

class VFlatInterpolator final : public IInterpolator
{
private:
    VColorARGB Color;

public:
    virtual ~VFlatInterpolator() = default;

    virtual void Start(const VPolyFace& Poly, const i32 InVtxIndices[3]) override
    {
        Color = Poly.LitColor[0];
    }

    virtual VColorARGB ComputePixel() override
    {
        return Color;
    }
};
