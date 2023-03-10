#pragma once

#include "Engine/Graphics/Interpolators/IInterpolator.h"

namespace Volition
{

class VAlphaInterpolator final : public IInterpolator
{
private:
    i32 Alpha;

public:
    virtual ~VAlphaInterpolator() = default;

    virtual void Start() override
    {
        Alpha = InterpolationContext->LitColor[0].A;
    }

    virtual void ProcessPixel() override
    {
        VColorARGB Pixel = InterpolationContext->Pixel;
        VColorARGB BufferPixel = InterpolationContext->Buffer[
            InterpolationContext->Y * InterpolationContext->BufferPitch + InterpolationContext->X
        ];

        InterpolationContext->Pixel = MAP_XRGB32(
            ( (Alpha * Pixel.R) + ((255 - Alpha) * BufferPixel.R) ) >> 8,
            ( (Alpha * Pixel.G) + ((255 - Alpha) * BufferPixel.G) ) >> 8,
            ( (Alpha * Pixel.B) + ((255 - Alpha) * BufferPixel.B) ) >> 8
        );
    }
};

}