#include "Engine/Graphics/Rendering/InterpolationContext.h"
#include "Engine/Graphics/Interpolators/GouraudInterpolator.h"

namespace Volition
{

static void StartFun(VGouraudInterpolator* Self)
{
    for (i32f i = 0; i < 3; ++i)
    {
        Self->RVtx[i] = IntToFx16(Self->InterpolationContext->LitColor[i].R);
        Self->GVtx[i] = IntToFx16(Self->InterpolationContext->LitColor[i].G);
        Self->BVtx[i] = IntToFx16(Self->InterpolationContext->LitColor[i].B);
    }
}

static void ComputeYStartsAndDeltasLeftFun(VGouraudInterpolator* Self, i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx)
{
    Self->RLeft = (Self->RVtx[LeftStartVtx]);
    Self->GLeft = (Self->GVtx[LeftStartVtx]);
    Self->BLeft = (Self->BVtx[LeftStartVtx]);

    Self->RDeltaLeftByY = (Self->RVtx[LeftEndVtx] - Self->RVtx[LeftStartVtx]) / YDiffLeft;
    Self->GDeltaLeftByY = (Self->GVtx[LeftEndVtx] - Self->GVtx[LeftStartVtx]) / YDiffLeft;
    Self->BDeltaLeftByY = (Self->BVtx[LeftEndVtx] - Self->BVtx[LeftStartVtx]) / YDiffLeft;
}

static void ComputeYStartsAndDeltasRightFun(VGouraudInterpolator* Self, i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx)
{
    Self->RRight = (Self->RVtx[RightStartVtx]);
    Self->GRight = (Self->GVtx[RightStartVtx]);
    Self->BRight = (Self->BVtx[RightStartVtx]);

    Self->RDeltaRightByY = (Self->RVtx[RightEndVtx] - Self->RVtx[RightStartVtx]) / YDiffRight;
    Self->GDeltaRightByY = (Self->GVtx[RightEndVtx] - Self->GVtx[RightStartVtx]) / YDiffRight;
    Self->BDeltaRightByY = (Self->BVtx[RightEndVtx] - Self->BVtx[RightStartVtx]) / YDiffRight;
}

static void SwapLeftRightFun(VGouraudInterpolator* Self)
{
    i32 TempInt;

    VLN_SWAP(Self->RDeltaLeftByY, Self->RDeltaRightByY, TempInt);
    VLN_SWAP(Self->GDeltaLeftByY, Self->GDeltaRightByY, TempInt);
    VLN_SWAP(Self->BDeltaLeftByY, Self->BDeltaRightByY, TempInt);

    VLN_SWAP(Self->RLeft, Self->RRight, TempInt);
    VLN_SWAP(Self->GLeft, Self->GRight, TempInt);
    VLN_SWAP(Self->BLeft, Self->BRight, TempInt);

    VLN_SWAP(Self->RVtx[Self->InterpolationContext->VtxIndices[1]], Self->RVtx[Self->InterpolationContext->VtxIndices[2]], TempInt);
    VLN_SWAP(Self->GVtx[Self->InterpolationContext->VtxIndices[1]], Self->GVtx[Self->InterpolationContext->VtxIndices[2]], TempInt);
    VLN_SWAP(Self->BVtx[Self->InterpolationContext->VtxIndices[1]], Self->BVtx[Self->InterpolationContext->VtxIndices[2]], TempInt);
}

static void ComputeXStartsAndDeltasFun(VGouraudInterpolator* Self, i32 XDiff, fx28 ZLeft, fx28 ZRight)
{
    Self->R = Self->RLeft + Fx16RoundUp;
    Self->G = Self->GLeft + Fx16RoundUp;
    Self->B = Self->BLeft + Fx16RoundUp;

    if (XDiff > 0)
    {
        Self->RDeltaByX = (Self->RRight - Self->RLeft) / XDiff;
        Self->GDeltaByX = (Self->GRight - Self->GLeft) / XDiff;
        Self->BDeltaByX = (Self->BRight - Self->BLeft) / XDiff;
    }
    else
    {
        Self->RDeltaByX = (Self->RRight - Self->RLeft);
        Self->GDeltaByX = (Self->GRight - Self->GLeft);
        Self->BDeltaByX = (Self->BRight - Self->BLeft);
    }
}

static void ProcessPixelFun(VGouraudInterpolator* Self)
{
    const VColorARGB Pixel = Self->InterpolationContext->Pixel;

    Self->InterpolationContext->Pixel = MAP_XRGB32(
        (Fx16ToInt(Self->R) * Pixel.R) >> 8,
        (Fx16ToInt(Self->G) * Pixel.G) >> 8,
        (Fx16ToInt(Self->B) * Pixel.B) >> 8
    );
}

static void InterpolateXFun(VGouraudInterpolator* Self, i32 X)
{
    Self->R += Self->RDeltaByX * X;
    Self->G += Self->GDeltaByX * X;
    Self->B += Self->BDeltaByX * X;
}

static void InterpolateYLeftFun(VGouraudInterpolator* Self, i32 YLeft)
{
    Self->RLeft += Self->RDeltaLeftByY * YLeft;
    Self->GLeft += Self->GDeltaLeftByY * YLeft;
    Self->BLeft += Self->BDeltaLeftByY * YLeft;
}

static void InterpolateYRightFun(VGouraudInterpolator* Self, i32 YRight)
{
    Self->RRight += Self->RDeltaRightByY * YRight;
    Self->GRight += Self->GDeltaRightByY * YRight;
    Self->BRight += Self->BDeltaRightByY * YRight;
}

VGouraudInterpolator::VGouraudInterpolator()
{
    Start = (StartType)StartFun;
    ComputeYStartsAndDeltasLeft = (ComputeYStartsAndDeltasLeftType)ComputeYStartsAndDeltasLeftFun;
    ComputeYStartsAndDeltasRight = (ComputeYStartsAndDeltasRightType)ComputeYStartsAndDeltasRightFun;
    SwapLeftRight = (SwapLeftRightType)SwapLeftRightFun;
    ComputeXStartsAndDeltas = (ComputeXStartsAndDeltasType)ComputeXStartsAndDeltasFun;
    ProcessPixel = (ProcessPixelType)ProcessPixelFun;
    InterpolateX = (InterpolateXType)InterpolateXFun;
    InterpolateYLeft = (InterpolateYLeftType)InterpolateYLeftFun;
    InterpolateYRight = (InterpolateYRightType)InterpolateYRightFun;
}

}
