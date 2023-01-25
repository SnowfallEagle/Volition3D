#pragma once

#include "Graphics/Interpolator/IInterpolator.h"

class VFlatInterpolator final : public IInterpolator
{
private:
    VColorARGB Color;

public:
    virtual ~VFlatInterpolator() = default;

    virtual void Start(const u32* Buffer, i32 Pitch, const VPolyFace& Poly, const i32 InVtxIndices[3]) override
    {
        Color = Poly.LitColor[0];
    }

    virtual VColorARGB ProcessPixel(VColorARGB Pixel, i32f X, i32f Y) override
    {
        return MAP_XRGB32(
            (Color.R * Pixel.R) >> 8,
            (Color.G * Pixel.G) >> 8,
            (Color.B * Pixel.B) >> 8
        );
    }
};
