#pragma once

#include "Engine/Graphics/Interpolators/IInterpolator.h"
#include "Engine/Math/Fixed16.h"

class VAffineTextureInterpolator final : public IInterpolator
{
private:
    fx16 UVtx[3], VVtx[3];

    fx16 U, V;

    fx16 ULeft, VLeft;
    fx16 URight, VRight;

    fx16 UDeltaLeftByY, VDeltaLeftByY;
    fx16 UDeltaRightByY, VDeltaRightByY;

    fx16 UDeltaByX, VDeltaByX;

    const u32* TextureBuffer;
    i32 TexturePitch;

public:
    virtual ~VAffineTextureInterpolator() = default;

    virtual void Start() override
    {
        const VSurface* Texture = &InterpolationContext->Material->Texture.Get(InterpolationContext->MipMappingLevel);
        VL_ASSERT(Texture);

        TextureBuffer = Texture->GetBuffer();
        TexturePitch = Texture->GetPitch();
        f32 TextureSize = (f32)Texture->GetWidth();

        for (i32f I = 0; I < 3; ++I)
        {
            UVtx[I] = IntToFx16((i32)(InterpolationContext->Vtx[I].U * TextureSize + 0.5f));
            VVtx[I] = IntToFx16((i32)(InterpolationContext->Vtx[I].V * TextureSize + 0.5f));
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

        VL_SWAP(UVtx[InterpolationContext->VtxIndices[1]], UVtx[InterpolationContext->VtxIndices[2]], TempInt);
        VL_SWAP(VVtx[InterpolationContext->VtxIndices[1]], VVtx[InterpolationContext->VtxIndices[2]], TempInt);
    }

    virtual void ComputeXStartsAndDeltas(i32 XDiff, fx28 ZLeft, fx28 ZRight) override
    {
        U = ULeft;
        V = VLeft;

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

    virtual void ProcessPixel() override
    {
        VColorARGB TextureColor = TextureBuffer[Fx16ToInt(V) * TexturePitch + Fx16ToInt(U)];
        VColorARGB Pixel = InterpolationContext->Pixel;

        InterpolationContext->Pixel = MAP_XRGB32(
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
};
