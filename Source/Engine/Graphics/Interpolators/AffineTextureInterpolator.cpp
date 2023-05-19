#include "Engine/Graphics/Rendering/InterpolationContext.h"
#include "Engine/Graphics/Interpolators/AffineTextureInterpolator.h"

namespace Volition
{

static void StartFun(VAffineTextureInterpolator* Self)
{
    const VSurface* Texture = &Self->InterpolationContext->Material->Texture.Get(Self->InterpolationContext->MipMappingLevel);
    VLN_ASSERT(Texture);

    Self->TextureBuffer = Texture->GetBuffer();
    Self->TexturePitch = Texture->GetPitch();
    const VVector2 TextureSize = { (f32)Texture->GetWidth(), (f32)Texture->GetHeight() };

    for (i32f i = 0; i < 3; ++i)
    {
        Self->UVtx[i] = FloatToFx16((Self->InterpolationContext->Vtx[i].U * TextureSize.X));
        Self->VVtx[i] = FloatToFx16((Self->InterpolationContext->Vtx[i].V * TextureSize.Y));
    }
}

static void ComputeYStartsAndDeltasLeftFun(VAffineTextureInterpolator* Self, i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx)
{
    Self->ULeft = Self->UVtx[LeftStartVtx];
    Self->VLeft = Self->VVtx[LeftStartVtx];

    Self->UDeltaLeftByY = (Self->UVtx[LeftEndVtx] - Self->UVtx[LeftStartVtx]) / YDiffLeft;
    Self->VDeltaLeftByY = (Self->VVtx[LeftEndVtx] - Self->VVtx[LeftStartVtx]) / YDiffLeft;
}

static void ComputeYStartsAndDeltasRightFun(VAffineTextureInterpolator* Self, i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx)
{
    Self->URight = Self->UVtx[RightStartVtx];
    Self->VRight = Self->VVtx[RightStartVtx];

    Self->UDeltaRightByY = (Self->UVtx[RightEndVtx] - Self->UVtx[RightStartVtx]) / YDiffRight;
    Self->VDeltaRightByY = (Self->VVtx[RightEndVtx] - Self->VVtx[RightStartVtx]) / YDiffRight;
}

static void SwapLeftRightFun(VAffineTextureInterpolator* Self)
{
    i32 TempInt;

    VLN_SWAP(Self->UDeltaLeftByY, Self->UDeltaRightByY, TempInt);
    VLN_SWAP(Self->VDeltaLeftByY, Self->VDeltaRightByY, TempInt);

    VLN_SWAP(Self->ULeft, Self->URight, TempInt);
    VLN_SWAP(Self->VLeft, Self->VRight, TempInt);

    VLN_SWAP(Self->UVtx[Self->InterpolationContext->VtxIndices[1]], Self->UVtx[Self->InterpolationContext->VtxIndices[2]], TempInt);
    VLN_SWAP(Self->VVtx[Self->InterpolationContext->VtxIndices[1]], Self->VVtx[Self->InterpolationContext->VtxIndices[2]], TempInt);
}

static void ComputeXStartsAndDeltasFun(VAffineTextureInterpolator* Self, i32 XDiff, fx28 ZLeft, fx28 ZRight)
{
    Self->U = Self->ULeft;
    Self->V = Self->VLeft;

    if (XDiff > 0)
    {
        Self->UDeltaByX = (Self->URight - Self->ULeft) / XDiff;
        Self->VDeltaByX = (Self->VRight - Self->VLeft) / XDiff;
    }
    else
    {
        Self->UDeltaByX = (Self->URight - Self->ULeft);
        Self->VDeltaByX = (Self->VRight - Self->VLeft);
    }
}

static void ProcessPixelFun(VAffineTextureInterpolator* Self)
{
    const VColorARGB TextureColor = Self->TextureBuffer[Fx16ToInt(Self->V) * Self->TexturePitch + Fx16ToInt(Self->U)];
    const VColorARGB Pixel = Self->InterpolationContext->Pixel;

    Self->InterpolationContext->Pixel = MAP_XRGB32(
        (TextureColor.R * Pixel.R) >> 8,
        (TextureColor.G * Pixel.G) >> 8,
        (TextureColor.B * Pixel.B) >> 8
    );
}

static void InterpolateXFun(VAffineTextureInterpolator* Self, i32 X)
{
    Self->U += Self->UDeltaByX * X;
    Self->V += Self->VDeltaByX * X;
}

static void InterpolateYLeftFun(VAffineTextureInterpolator* Self, i32 YLeft)
{
    Self->ULeft += Self->UDeltaLeftByY * YLeft;
    Self->VLeft += Self->VDeltaLeftByY * YLeft;
}

static void InterpolateYRightFun(VAffineTextureInterpolator* Self, i32 YRight)
{
    Self->URight += Self->UDeltaRightByY * YRight;
    Self->VRight += Self->VDeltaRightByY * YRight;
}

VAffineTextureInterpolator::VAffineTextureInterpolator()
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
