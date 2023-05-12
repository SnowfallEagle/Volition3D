#include "Engine/Graphics/Rendering/InterpolationContext.h"
#include "Engine/Graphics/Interpolators/LinearPiecewiseTextureInterpolator.h"

namespace Volition
{

static void StartFun(VLinearPiecewiseTextureInterpolator* Self)
{
    const VSurface* Texture = &Self->InterpolationContext->Material->Texture.Get(Self->InterpolationContext->MipMappingLevel);
    VLN_ASSERT(Texture);

    Self->TextureBuffer = Texture->GetBuffer();
    Self->TexturePitch = Texture->GetPitch();
    const VVector2 TextureSize = { (f32)Texture->GetWidth(), (f32)Texture->GetHeight() };

    for (i32f i = 0; i < 3; ++i)
    {
        Self->UVtx[i] =
            IntToFx22((i32)(Self->InterpolationContext->Vtx[i].U * TextureSize.X + 0.5f)) /
                (i32)(Self->InterpolationContext->Vtx[i].Z + 0.5f);
        Self->VVtx[i] =
            IntToFx22((i32)(Self->InterpolationContext->Vtx[i].V * TextureSize.Y + 0.5f)) /
                (i32)(Self->InterpolationContext->Vtx[i].Z + 0.5f);
    }
}

static void ComputeYStartsAndDeltasLeftFun(VLinearPiecewiseTextureInterpolator* Self, i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx)
{
    Self->ULeft = (Self->UVtx[LeftStartVtx]);
    Self->VLeft = (Self->VVtx[LeftStartVtx]);

    Self->UDeltaLeftByY = (Self->UVtx[LeftEndVtx] - Self->UVtx[LeftStartVtx]) / YDiffLeft;
    Self->VDeltaLeftByY = (Self->VVtx[LeftEndVtx] - Self->VVtx[LeftStartVtx]) / YDiffLeft;
}

static void ComputeYStartsAndDeltasRightFun(VLinearPiecewiseTextureInterpolator* Self, i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx)
{
    Self->URight = (Self->UVtx[RightStartVtx]);
    Self->VRight = (Self->VVtx[RightStartVtx]);

    Self->UDeltaRightByY = (Self->UVtx[RightEndVtx] - Self->UVtx[RightStartVtx]) / YDiffRight;
    Self->VDeltaRightByY = (Self->VVtx[RightEndVtx] - Self->VVtx[RightStartVtx]) / YDiffRight;
}

static void SwapLeftRightFun(VLinearPiecewiseTextureInterpolator* Self)
{
    i32 TempInt;

    VLN_SWAP(Self->UDeltaLeftByY, Self->UDeltaRightByY, TempInt);
    VLN_SWAP(Self->VDeltaLeftByY, Self->VDeltaRightByY, TempInt);

    VLN_SWAP(Self->ULeft, Self->URight, TempInt);
    VLN_SWAP(Self->VLeft, Self->VRight, TempInt);

    VLN_SWAP(Self->UVtx[Self->InterpolationContext->VtxIndices[1]], Self->UVtx[Self->InterpolationContext->VtxIndices[2]], TempInt);
    VLN_SWAP(Self->VVtx[Self->InterpolationContext->VtxIndices[1]], Self->VVtx[Self->InterpolationContext->VtxIndices[2]], TempInt);
}

static void ComputeXStartsAndDeltasFun(VLinearPiecewiseTextureInterpolator* Self, i32 XDiff, fx28 ZLeft, fx28 ZRight)
{
    const fx22 ULeftDivZ = ((Self->ULeft << (Fx28Shift - Fx22Shift)) / (ZLeft >> 6)) << 16;
    const fx22 URightDivZ = ((Self->URight << (Fx28Shift - Fx22Shift)) / (ZRight >> 6)) << 16;

    const fx22 VLeftDivZ = ((Self->VLeft << (Fx28Shift - Fx22Shift)) / (ZLeft >> 6)) << 16;
    const fx22 VRightDivZ = ((Self->VRight << (Fx28Shift - Fx22Shift)) / (ZRight >> 6)) << 16;

    Self->U = ULeftDivZ;
    Self->V = VLeftDivZ;

    if (XDiff > 0)
    {
        Self->UDeltaByX = (URightDivZ - ULeftDivZ) / XDiff;
        Self->VDeltaByX = (VRightDivZ - VLeftDivZ) / XDiff;
    }
    else
    {
        Self->UDeltaByX = (URightDivZ - ULeftDivZ);
        Self->VDeltaByX = (VRightDivZ - VLeftDivZ);
    }
}

static void InterpolateXFun(VLinearPiecewiseTextureInterpolator* Self, i32 X)
{
    Self->U += Self->UDeltaByX * X;
    Self->V += Self->VDeltaByX * X;
}

static void InterpolateYLeftFun(VLinearPiecewiseTextureInterpolator* Self, i32 YLeft)
{
    Self->ULeft += Self->UDeltaLeftByY * YLeft;
    Self->VLeft += Self->VDeltaLeftByY * YLeft;
}

static void InterpolateYRightFun(VLinearPiecewiseTextureInterpolator* Self, i32 YRight)
{
    Self->URight += Self->UDeltaRightByY * YRight;
    Self->VRight += Self->VDeltaRightByY * YRight;
}

static void ProcessPixelFun(VLinearPiecewiseTextureInterpolator* Self)
{
    const VColorARGB Pixel = Self->InterpolationContext->Pixel;
    const VColorARGB TextureColor = Self->TextureBuffer[Fx22ToInt(Self->V) * Self->TexturePitch + Fx22ToInt(Self->U)];

    Self->InterpolationContext->Pixel = MAP_XRGB32(
        (TextureColor.R * Pixel.R) >> 8,
        (TextureColor.G * Pixel.G) >> 8,
        (TextureColor.B * Pixel.B) >> 8
    );
}

VLinearPiecewiseTextureInterpolator::VLinearPiecewiseTextureInterpolator()
{
    Start = (StartType)StartFun;
    ComputeYStartsAndDeltasLeft = (ComputeYStartsAndDeltasLeftType)ComputeYStartsAndDeltasLeftFun;
    ComputeYStartsAndDeltasRight = (ComputeYStartsAndDeltasRightType)ComputeYStartsAndDeltasRightFun;
    SwapLeftRight = (SwapLeftRightType)SwapLeftRightFun;
    ComputeXStartsAndDeltas = (ComputeXStartsAndDeltasType)ComputeXStartsAndDeltasFun;
    InterpolateX = (InterpolateXType)InterpolateXFun;
    InterpolateYLeft = (InterpolateYLeftType)InterpolateYLeftFun;
    InterpolateYRight = (InterpolateYRightType)InterpolateYRightFun;
    ProcessPixel = (ProcessPixelType)ProcessPixelFun;
}

}

