#pragma once

#include "Engine/Graphics/Interpolators/IInterpolator.h"

class VAlphaInterpolator final : public IInterpolator
{
private:
    const u32* Buffer;
    i32 Pitch;
    i32 Alpha;

public:
    virtual ~VAlphaInterpolator() = default;

    virtual void Start(const u32* InBuffer, i32 InPitch, const VPolyFace& Poly, const i32 InVtxIndices[3]) override
    {
        Buffer = InBuffer;
        Pitch = InPitch;
        Alpha = Poly.LitColor[0].A;
    }

    virtual VColorARGB ProcessPixel(VColorARGB Pixel, i32f X, i32f Y, fx28 Z) override
    {
        VColorARGB ScreenColor = Buffer[Y * Pitch + X];

        return MAP_XRGB32(
            ( (Alpha * Pixel.R) + ((255 - Alpha) * ScreenColor.R) ) >> 8,
            ( (Alpha * Pixel.G) + ((255 - Alpha) * ScreenColor.G) ) >> 8,
            ( (Alpha * Pixel.B) + ((255 - Alpha) * ScreenColor.B) ) >> 8
        );
    }
};
