#pragma once

#include "Graphics/Interpolator/IInterpolator.h"

class VGouraudInterpolator final : public IInterpolator
{
private:
    i32 VtxIndices[3];
    i32 RVtx[3], GVtx[3], BVtx[3];

    fx16 R, G, B;

    fx16 RLeft, GLeft, BLeft;
    fx16 RRight, GRight, BRight;

    fx16 RDeltaLeftByY, GDeltaLeftByY, BDeltaLeftByY;
    fx16 RDeltaRightByY, GDeltaRightByY, BDeltaRightByY;

    fx16 RDeltaByX, GDeltaByX, BDeltaByX;

public:
    virtual ~VGouraudInterpolator() = default;

    virtual void Start(const VPolyFace& Poly, const i32 InVtxIndices[3]) override
    {
        for (i32f I = 0; I < 3; ++I)
        {
            VtxIndices[I] = InVtxIndices[I];
            RVtx[I] = Poly.LitColor[I].R;
            GVtx[I] = Poly.LitColor[I].G;
            BVtx[I] = Poly.LitColor[I].B;
        }
    }

    virtual void ComputeYStartsAndDeltasLeft(i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx) override
    {
        RLeft = IntToFx16(RVtx[LeftStartVtx]);
        GLeft = IntToFx16(GVtx[LeftStartVtx]);
        BLeft = IntToFx16(BVtx[LeftStartVtx]);

        RDeltaLeftByY = IntToFx16(RVtx[LeftEndVtx] - RVtx[LeftStartVtx]) / YDiffLeft;
        GDeltaLeftByY = IntToFx16(GVtx[LeftEndVtx] - GVtx[LeftStartVtx]) / YDiffLeft;
        BDeltaLeftByY = IntToFx16(BVtx[LeftEndVtx] - BVtx[LeftStartVtx]) / YDiffLeft;
    }
    virtual void ComputeYStartsAndDeltasRight(i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx) override
    {
        RRight = IntToFx16(RVtx[RightStartVtx]);
        GRight = IntToFx16(GVtx[RightStartVtx]);
        BRight = IntToFx16(BVtx[RightStartVtx]);

        RDeltaRightByY = IntToFx16(RVtx[RightEndVtx] - RVtx[RightStartVtx]) / YDiffRight;
        GDeltaRightByY = IntToFx16(GVtx[RightEndVtx] - GVtx[RightStartVtx]) / YDiffRight;
        BDeltaRightByY = IntToFx16(BVtx[RightEndVtx] - BVtx[RightStartVtx]) / YDiffRight;
    }

    virtual void SwapLeftRight() override
    {
        i32 TempInt;

        SWAP(RDeltaLeftByY, RDeltaRightByY, TempInt);
        SWAP(GDeltaLeftByY, GDeltaRightByY, TempInt);
        SWAP(BDeltaLeftByY, BDeltaRightByY, TempInt);

        SWAP(RLeft, RRight, TempInt);
        SWAP(GLeft, GRight, TempInt);
        SWAP(BLeft, BRight, TempInt);

        SWAP(RVtx[VtxIndices[1]], RVtx[VtxIndices[2]], TempInt);
        SWAP(GVtx[VtxIndices[1]], GVtx[VtxIndices[2]], TempInt);
        SWAP(BVtx[VtxIndices[1]], BVtx[VtxIndices[2]], TempInt);
    }

    virtual void ComputeXStartsAndDeltas(i32 XDiff) override
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

    virtual VColorARGB ComputePixel() override
    {
        return MAP_XRGB32(
            Fx16ToInt(R),
            Fx16ToInt(G),
            Fx16ToInt(B)
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
