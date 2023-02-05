#pragma once

#include "Engine/Graphics/Interpolators/IInterpolator.h"

class VFlatInterpolator final : public IInterpolator
{
private:
    VColorARGB Color;

public:
    virtual ~VFlatInterpolator() = default;

    virtual void Start() override
    {
        Color = InterpolationContext->Poly->LitColor[0];
    }

    virtual void ProcessPixel() override
    {
        InterpolationContext->Pixel = MAP_XRGB32(
            (Color.R * InterpolationContext->Pixel.R) >> 8,
            (Color.G * InterpolationContext->Pixel.G) >> 8,
            (Color.B * InterpolationContext->Pixel.B) >> 8
        );
    }
};
