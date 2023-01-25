#pragma once

#include "Graphics/Interpolator/IInterpolator.h"

class VAlphaInterpolator final : public IInterpolator
{
private:
    const u32* Buffer;
    i32 Pitch;
    VColorARGB AlphaColor;

public:
    virtual ~VAlphaInterpolator() = default;

    virtual void Start(const u32* InBuffer, i32 InPitch, const VPolyFace& Poly, const i32 InVtxIndices[3]) override
    {
        Buffer = InBuffer;
        Pitch = InPitch;

        i32 AlphaComponent = (0xFF * Poly.LitColor[0].A) >> 8;
        AlphaColor = MAP_XRGB32(AlphaComponent, AlphaComponent, AlphaComponent);
    }

    /* TODO(sean):
        void ProccessPixel(VColorARGB& Pixel)
        or
        VColorARGB ProccessPixel(VColorARGB Pixel)
    */
    virtual VColorARGB ComputePixel() override
    {
        return MAP_XRGB32(0xFF, 0xFF, 0xFF);
    }
};
