#pragma once

#include "Graphics/Interpolator/IInterpolator.h"

class VTextureInterpolator final : public IInterpolator
{
private:
    i32 VtxIndices[3];
    i32 UVtx[3], VVtx[3];

    fx16 U, V;

    fx16 ULeft, VLeft;
    fx16 URight, VRight;

    fx16 UDeltaLeftByY, VDeltaLeftByY;
    fx16 UDeltaRightByY, VDeltaRightByY;

    fx16 UDeltaByX, VDeltaByX;

    VSurface* Texture;
    u32* TextureBuffer;
    i32 TexturePitch;

public:
    virtual ~VTextureInterpolator() = default;

    virtual void Start(const u32* Buffer, i32 Pitch, const VPolyFace& Poly, const i32 InVtxIndices[3]) override
    {
        ASSERT(Poly.Texture);

        Texture = Poly.Texture;
        Texture->Lock(TextureBuffer, TexturePitch);

        for (i32f I = 0; I < 3; ++I)
        {
            VtxIndices[I] = InVtxIndices[I];
            // TODO(sean): Possible optmization: We can convert UVtxs and VVtxs to fx16 right here
            UVtx[I] = (i32)(Poly.TransVtx[I].U + 0.5f);
            VVtx[I] = (i32)(Poly.TransVtx[I].V + 0.5f);
        }
    }

    virtual void ComputeYStartsAndDeltasLeft(i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx) override
    {
        ULeft = IntToFx16(UVtx[LeftStartVtx]);
        VLeft = IntToFx16(VVtx[LeftStartVtx]);

        UDeltaLeftByY = IntToFx16(UVtx[LeftEndVtx] - UVtx[LeftStartVtx]) / YDiffLeft;
        VDeltaLeftByY = IntToFx16(VVtx[LeftEndVtx] - VVtx[LeftStartVtx]) / YDiffLeft;
    }
    virtual void ComputeYStartsAndDeltasRight(i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx) override
    {
        URight = IntToFx16(UVtx[RightStartVtx]);
        VRight = IntToFx16(VVtx[RightStartVtx]);

        UDeltaRightByY = IntToFx16(UVtx[RightEndVtx] - UVtx[RightStartVtx]) / YDiffRight;
        VDeltaRightByY = IntToFx16(VVtx[RightEndVtx] - VVtx[RightStartVtx]) / YDiffRight;
    }

    virtual void SwapLeftRight() override
    {
        i32 TempInt;

        SWAP(UDeltaLeftByY, UDeltaRightByY, TempInt);
        SWAP(VDeltaLeftByY, VDeltaRightByY, TempInt);

        SWAP(ULeft, URight, TempInt);
        SWAP(VLeft, VRight, TempInt);

        SWAP(UVtx[VtxIndices[1]], UVtx[VtxIndices[2]], TempInt);
        SWAP(VVtx[VtxIndices[1]], VVtx[VtxIndices[2]], TempInt);
    }

    virtual void ComputeXStartsAndDeltas(i32 XDiff) override
    {
        U = ULeft + Fx16RoundUp;
        V = VLeft + Fx16RoundUp;

        if (XDiff > 0)
        {
            UDeltaByX = (URight - ULeft) / XDiff;
            VDeltaByX = (VRight - VLeft) / XDiff;
        }
        else
        {
            UDeltaByX = (URight - ULeft);
            VDeltaByX = (VRight - VLeft);
        }
    }

    virtual VColorARGB ProcessPixel(VColorARGB Pixel, i32f X, i32f Y) override
    {
        VColorARGB TextureColor = TextureBuffer[Fx16ToInt(V) * TexturePitch + Fx16ToInt(U)];
        return MAP_XRGB32(
            (TextureColor.R * Pixel.R) >> 8,
            (TextureColor.G * Pixel.G) >> 8,
            (TextureColor.B * Pixel.B) >> 8
        );
    }

    virtual void InterpolateX(i32 X) override
    {
        U += UDeltaByX * X;
        V += VDeltaByX * X;
    }

    virtual void InterpolateYLeft(i32 YLeft) override
    {
        ULeft += UDeltaLeftByY * YLeft;
        VLeft += VDeltaLeftByY * YLeft;
    }
    virtual void InterpolateYRight(i32 YRight) override
    {
        URight += UDeltaRightByY * YRight;
        VRight += VDeltaRightByY * YRight;
    }

    virtual void End() override
    {
        Texture->Unlock();
    }
};
