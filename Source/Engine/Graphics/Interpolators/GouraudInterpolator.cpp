#include "Engine/Graphics/Rendering/InterpolationContext.h"
#include "Engine/Graphics/Interpolators/GouraudInterpolator.h"

namespace Volition
{

void VGouraudInterpolator::Start()
{
    for (i32f I = 0; I < 3; ++I)
    {
        RVtx[I] = IntToFx16(InterpolationContext->LitColor[I].R);
        GVtx[I] = IntToFx16(InterpolationContext->LitColor[I].G);
        BVtx[I] = IntToFx16(InterpolationContext->LitColor[I].B);
    }
}

void VGouraudInterpolator::ComputeYStartsAndDeltasLeft(i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx)
{
    RLeft = (RVtx[LeftStartVtx]);
    GLeft = (GVtx[LeftStartVtx]);
    BLeft = (BVtx[LeftStartVtx]);

    RDeltaLeftByY = (RVtx[LeftEndVtx] - RVtx[LeftStartVtx]) / YDiffLeft;
    GDeltaLeftByY = (GVtx[LeftEndVtx] - GVtx[LeftStartVtx]) / YDiffLeft;
    BDeltaLeftByY = (BVtx[LeftEndVtx] - BVtx[LeftStartVtx]) / YDiffLeft;
}

void VGouraudInterpolator::ComputeYStartsAndDeltasRight(i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx)
{
    RRight = (RVtx[RightStartVtx]);
    GRight = (GVtx[RightStartVtx]);
    BRight = (BVtx[RightStartVtx]);

    RDeltaRightByY = (RVtx[RightEndVtx] - RVtx[RightStartVtx]) / YDiffRight;
    GDeltaRightByY = (GVtx[RightEndVtx] - GVtx[RightStartVtx]) / YDiffRight;
    BDeltaRightByY = (BVtx[RightEndVtx] - BVtx[RightStartVtx]) / YDiffRight;
}

void VGouraudInterpolator::SwapLeftRight()
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

void VGouraudInterpolator::ComputeXStartsAndDeltas(i32 XDiff, fx28 ZLeft, fx28 ZRight)
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

void VGouraudInterpolator::ProcessPixel()
{
    const VColorARGB Pixel = InterpolationContext->Pixel;

    InterpolationContext->Pixel = MAP_XRGB32(
        (Fx16ToInt(R) * Pixel.R) >> 8,
        (Fx16ToInt(G) * Pixel.G) >> 8,
        (Fx16ToInt(B) * Pixel.B) >> 8
    );
}

void VGouraudInterpolator::InterpolateX(i32 X)
{
    R += RDeltaByX * X;
    G += GDeltaByX * X;
    B += BDeltaByX * X;
}

void VGouraudInterpolator::InterpolateYLeft(i32 YLeft)
{
    RLeft += RDeltaLeftByY * YLeft;
    GLeft += GDeltaLeftByY * YLeft;
    BLeft += BDeltaLeftByY * YLeft;
}

void VGouraudInterpolator::InterpolateYRight(i32 YRight)
{
    RRight += RDeltaRightByY * YRight;
    GRight += GDeltaRightByY * YRight;
    BRight += BDeltaRightByY * YRight;
}

}
