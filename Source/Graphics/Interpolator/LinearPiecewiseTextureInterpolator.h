#pragma once

#include "Graphics/Interpolator/IInterpolator.h"
#include "Math/Fixed22.h"

class VLinearPiecewiseTextureInterpolator final : public IInterpolator
{
private:
    i32 VtxIndices[3];
    fx22 UVtx[3], VVtx[3];

    fx22 U, V;

    fx22 ULeft, VLeft;
    fx22 URight, VRight;

    fx22 UDeltaLeftByY, VDeltaLeftByY;
    fx22 UDeltaRightByY, VDeltaRightByY;

    fx22 UDeltaByX, VDeltaByX;

    VSurface* Texture;
    u32* TextureBuffer;
    i32 TexturePitch;

public:
    virtual ~VLinearPiecewiseTextureInterpolator() = default;

    virtual void Start(const u32* Buffer, i32 Pitch, const VPolyFace& Poly, const i32 InVtxIndices[3]) override
    {
        VL_ASSERT(Poly.Texture);

        Texture = Poly.Texture;
        Texture->Lock(TextureBuffer, TexturePitch);

        for (i32f I = 0; I < 3; ++I)
        {
            VtxIndices[I] = InVtxIndices[I];
            UVtx[I] = IntToFx22((i32)(Poly.TransVtx[I].U + 0.5f)) / (i32)(Poly.TransVtx[I].Z + 0.5f);
            VVtx[I] = IntToFx22((i32)(Poly.TransVtx[I].V + 0.5f)) / (i32)(Poly.TransVtx[I].Z + 0.5f);
        }
    }

    virtual void ComputeYStartsAndDeltasLeft(i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx) override
    {
        ULeft = (UVtx[LeftStartVtx]);
        VLeft = (VVtx[LeftStartVtx]);

        UDeltaLeftByY = (UVtx[LeftEndVtx] - UVtx[LeftStartVtx]) / YDiffLeft;
        VDeltaLeftByY = (VVtx[LeftEndVtx] - VVtx[LeftStartVtx]) / YDiffLeft;
    }
    virtual void ComputeYStartsAndDeltasRight(i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx) override
    {
        URight = (UVtx[RightStartVtx]);
        VRight = (VVtx[RightStartVtx]);

        UDeltaRightByY = (UVtx[RightEndVtx] - UVtx[RightStartVtx]) / YDiffRight;
        VDeltaRightByY = (VVtx[RightEndVtx] - VVtx[RightStartVtx]) / YDiffRight;
    }

    virtual void SwapLeftRight() override
    {
        i32 TempInt;

        VL_SWAP(UDeltaLeftByY, UDeltaRightByY, TempInt);
        VL_SWAP(VDeltaLeftByY, VDeltaRightByY, TempInt);

        VL_SWAP(ULeft, URight, TempInt);
        VL_SWAP(VLeft, VRight, TempInt);

        VL_SWAP(UVtx[VtxIndices[1]], UVtx[VtxIndices[2]], TempInt);
        VL_SWAP(VVtx[VtxIndices[1]], VVtx[VtxIndices[2]], TempInt);
    }

    virtual void ComputeXStartsAndDeltas(i32 XDiff, fx28 ZLeft, fx28 ZRight) override
    {
        fx22 ULeftDivZ  = ((ULeft  << (Fx28Shift - Fx22Shift)) / (ZLeft  >> 6)) << 16;
        fx22 URightDivZ = ((URight << (Fx28Shift - Fx22Shift)) / (ZRight >> 6)) << 16;

        fx22 VLeftDivZ  = ((VLeft  << (Fx28Shift - Fx22Shift)) / (ZLeft  >> 6)) << 16;
        fx22 VRightDivZ = ((VRight << (Fx28Shift - Fx22Shift)) / (ZRight >> 6)) << 16;

        U = ULeftDivZ;
        V = VLeftDivZ;

        if (XDiff > 0)
        {
            UDeltaByX = (URightDivZ  - ULeftDivZ) / XDiff;
            VDeltaByX = (VRightDivZ  - VLeftDivZ) / XDiff;
        }
        else
        {
            UDeltaByX = (URightDivZ - ULeftDivZ);
            VDeltaByX = (VRightDivZ  - VLeftDivZ);
        }
    }

    virtual VColorARGB ProcessPixel(VColorARGB Pixel, i32f X, i32f Y, fx28 Z) override
    {
        VColorARGB TextureColor = TextureBuffer[Fx22ToInt(V) * TexturePitch + Fx22ToInt(U)];

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
