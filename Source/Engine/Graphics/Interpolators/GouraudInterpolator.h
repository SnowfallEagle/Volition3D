#pragma once

#include "Engine/Graphics/Interpolators/IInterpolator.h"

class VGouraudInterpolator final : public IInterpolator
{
private:
    fx16 RVtx[3], GVtx[3], BVtx[3];

    fx16 R, G, B;

    fx16 RLeft, GLeft, BLeft;
    fx16 RRight, GRight, BRight;

    fx16 RDeltaLeftByY, GDeltaLeftByY, BDeltaLeftByY;
    fx16 RDeltaRightByY, GDeltaRightByY, BDeltaRightByY;

    fx16 RDeltaByX, GDeltaByX, BDeltaByX;

public:
    virtual ~VGouraudInterpolator() = default;

    virtual void Start() override
    {
        for (i32f I = 0; I < 3; ++I)
        {
            RVtx[I] = IntToFx16(InterpolationContext->LitColor[I].R);
            GVtx[I] = IntToFx16(InterpolationContext->LitColor[I].G);
            BVtx[I] = IntToFx16(InterpolationContext->LitColor[I].B);
        }
    }

    virtual void ComputeYStartsAndDeltasLeft(i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx) override
    {
        RLeft = (RVtx[LeftStartVtx]);
        GLeft = (GVtx[LeftStartVtx]);
        BLeft = (BVtx[LeftStartVtx]);

        RDeltaLeftByY = (RVtx[LeftEndVtx] - RVtx[LeftStartVtx]) / YDiffLeft;
        GDeltaLeftByY = (GVtx[LeftEndVtx] - GVtx[LeftStartVtx]) / YDiffLeft;
        BDeltaLeftByY = (BVtx[LeftEndVtx] - BVtx[LeftStartVtx]) / YDiffLeft;
    }
    virtual void ComputeYStartsAndDeltasRight(i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx) override
    {
        RRight = (RVtx[RightStartVtx]);
        GRight = (GVtx[RightStartVtx]);
        BRight = (BVtx[RightStartVtx]);

        RDeltaRightByY = (RVtx[RightEndVtx] - RVtx[RightStartVtx]) / YDiffRight;
        GDeltaRightByY = (GVtx[RightEndVtx] - GVtx[RightStartVtx]) / YDiffRight;
        BDeltaRightByY = (BVtx[RightEndVtx] - BVtx[RightStartVtx]) / YDiffRight;
    }

    virtual void SwapLeftRight() override
    {
        i32 TempInt;

        VLN_SWAP(RDeltaLeftByY, RDeltaRightByY, TempInt);
        VLN_SWAP(GDeltaLeftByY, GDeltaRightByY, TempInt);
        VLN_SWAP(BDeltaLeftByY, BDeltaRightByY, TempInt);

        VLN_SWAP(RLeft, RRight, TempInt);
        VLN_SWAP(GLeft, GRight, TempInt);
        VLN_SWAP(BLeft, BRight, TempInt);

        VLN_SWAP(RVtx[InterpolationContext->VtxIndices[1]], RVtx[InterpolationContext->VtxIndices[2]], TempInt);
        VLN_SWAP(GVtx[InterpolationContext->VtxIndices[1]], GVtx[InterpolationContext->VtxIndices[2]], TempInt);
        VLN_SWAP(BVtx[InterpolationContext->VtxIndices[1]], BVtx[InterpolationContext->VtxIndices[2]], TempInt);
    }

    virtual void ComputeXStartsAndDeltas(i32 XDiff, fx28 ZLeft, fx28 ZRight) override
    {
        R = RLeft + Fx16RoundUp;
        G = GLeft + Fx16RoundUp;
        B = BLeft + Fx16RoundUp;

        if (XDiff > 0)
        {
            RDeltaByX = (RRight - RLeft) / XDiff;
            GDeltaByX = (GRight - GLeft) / XDiff;
            BDeltaByX = (BRight - BLeft) / XDiff;
        }
        else
        {
            RDeltaByX = (RRight - RLeft);
            GDeltaByX = (GRight - GLeft);
            BDeltaByX = (BRight - BLeft);
        }
    }

    virtual void ProcessPixel() override
    {
        VColorARGB Pixel = InterpolationContext->Pixel;

        InterpolationContext->Pixel = MAP_XRGB32(
            (Fx16ToInt(R) * Pixel.R) >> 8,
            (Fx16ToInt(G) * Pixel.G) >> 8,
            (Fx16ToInt(B) * Pixel.B) >> 8
        );
    }

    virtual void InterpolateX(i32 X) override
    {
        R += RDeltaByX * X;
        G += GDeltaByX * X;
        B += BDeltaByX * X;
    }

    virtual void InterpolateYLeft(i32 YLeft) override
    {
        RLeft += RDeltaLeftByY * YLeft;
        GLeft += GDeltaLeftByY * YLeft;
        BLeft += BDeltaLeftByY * YLeft;
    }
    virtual void InterpolateYRight(i32 YRight) override
    {
        RRight += RDeltaRightByY * YRight;
        GRight += GDeltaRightByY * YRight;
        BRight += BDeltaRightByY * YRight;
    }
};
