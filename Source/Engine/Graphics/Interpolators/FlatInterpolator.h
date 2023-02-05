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
        Color = InterpolationContext->LitColor[0];
    }

    virtual void ProcessPixel() override
    {
        VColorARGB Pixel = InterpolationContext->Pixel;

        InterpolationContext->Pixel = MAP_XRGB32(
            (Color.R * Pixel.R) >> 8,
            (Color.G * Pixel.G) >> 8,
            (Color.B * Pixel.B) >> 8
        );
    }
};
