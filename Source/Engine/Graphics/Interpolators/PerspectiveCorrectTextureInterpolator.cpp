
#include "Engine/Graphics/Rendering/InterpolationContext.h"
#include "Engine/Graphics/Interpolators/PerspectiveCorrectTextureInterpolator.h"

namespace Volition
{

static void StartFun(VPerspectiveCorrectTextureInterpolator* Self)
{
    const VSurface* Texture = &Self->InterpolationContext->Material->Texture.Get(Self->InterpolationContext->MipMappingLevel);
    VLN_ASSERT(Texture);

    Self->TextureBuffer = Texture->GetBuffer();
    Self->TexturePitch = Texture->GetPitch();

    const VVector2 TextureSize = { (f32)Texture->GetWidth(), (f32)Texture->GetHeight() };
    Self->TextureSize = { (i32)TextureSize.X, (i32)TextureSize.Y };

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

static void ComputeYStartsAndDeltasLeftFun(VPerspectiveCorrectTextureInterpolator* Self, i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx)
{
    Self->ULeft = (Self->UVtx[LeftStartVtx]);
    Self->VLeft = (Self->VVtx[LeftStartVtx]);

    Self->UDeltaLeftByY = (Self->UVtx[LeftEndVtx] - Self->UVtx[LeftStartVtx]) / YDiffLeft;
    Self->VDeltaLeftByY = (Self->VVtx[LeftEndVtx] - Self->VVtx[LeftStartVtx]) / YDiffLeft;
}

static void ComputeYStartsAndDeltasRightFun(VPerspectiveCorrectTextureInterpolator* Self, i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx)
{
    Self->URight = (Self->UVtx[RightStartVtx]);
    Self->VRight = (Self->VVtx[RightStartVtx]);

    Self->UDeltaRightByY = (Self->UVtx[RightEndVtx] - Self->UVtx[RightStartVtx]) / YDiffRight;
    Self->VDeltaRightByY = (Self->VVtx[RightEndVtx] - Self->VVtx[RightStartVtx]) / YDiffRight;
}

static void SwapLeftRightFun(VPerspectiveCorrectTextureInterpolator* Self)
{
    i32 TempInt;

    VLN_SWAP(Self->UDeltaLeftByY, Self->UDeltaRightByY, TempInt);
    VLN_SWAP(Self->VDeltaLeftByY, Self->VDeltaRightByY, TempInt);

    VLN_SWAP(Self->ULeft, Self->URight, TempInt);
    VLN_SWAP(Self->VLeft, Self->VRight, TempInt);

    VLN_SWAP(Self->UVtx[Self->InterpolationContext->VtxIndices[1]], Self->UVtx[Self->InterpolationContext->VtxIndices[2]], TempInt);
    VLN_SWAP(Self->VVtx[Self->InterpolationContext->VtxIndices[1]], Self->VVtx[Self->InterpolationContext->VtxIndices[2]], TempInt);
}

static void ComputeXStartsAndDeltasFun(VPerspectiveCorrectTextureInterpolator* Self, i32 XDiff, fx28 ZLeft, fx28 ZRight)
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

static void ProcessPixelFun(VPerspectiveCorrectTextureInterpolator* Self)
{
    i32 U = (Self->U << (Fx28Shift - Fx22Shift)) / Self->InterpolationContext->Z;
    i32 V = (Self->V << (Fx28Shift - Fx22Shift)) / Self->InterpolationContext->Z;

    // @NOTE: Crash may occur if something happened with poly's Z
#if VLN_MODE == VLN_MODE_SAFE
    if (U > Self->TextureSize.X)
    {
        U = Self->TextureSize.X;
    }
    if (V > Self->TextureSize.Y)
    {
        V = Self->TextureSize.Y;
    }
#endif

    const VColorARGB TextureColor = Self->TextureBuffer[
        V * Self->TexturePitch +
        ((Self->U << (Fx28Shift - Fx22Shift)) / Self->InterpolationContext->Z)
    ];

    const VColorARGB Pixel = Self->InterpolationContext->Pixel;
    Self->InterpolationContext->Pixel = MAP_XRGB32(
        (TextureColor.R * Pixel.R) >> 8,
        (TextureColor.G * Pixel.G) >> 8,
        (TextureColor.B * Pixel.B) >> 8
    );
}

static void InterpolateXFun(VPerspectiveCorrectTextureInterpolator* Self, i32 X)
{
    Self->U += Self->UDeltaByX * X;
    Self->V += Self->VDeltaByX * X;
}

static void InterpolateYLeftFun(VPerspectiveCorrectTextureInterpolator* Self, i32 YLeft)
{
    Self->ULeft += Self->UDeltaLeftByY * YLeft;
    Self->VLeft += Self->VDeltaLeftByY * YLeft;
}

static void InterpolateYRightFun(VPerspectiveCorrectTextureInterpolator* Self, i32 YRight)
{
    Self->URight += Self->UDeltaRightByY * YRight;
    Self->VRight += Self->VDeltaRightByY * YRight;
}

VPerspectiveCorrectTextureInterpolator::VPerspectiveCorrectTextureInterpolator()
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
