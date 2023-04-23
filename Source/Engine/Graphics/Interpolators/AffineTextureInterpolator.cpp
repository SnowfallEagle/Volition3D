#include "Engine/Graphics/InterpolationContext.h"
#include "Engine/Graphics/Interpolators/AffineTextureInterpolator.h"

namespace Volition
{

void VAffineTextureInterpolator::Start()
{
    const VSurface* Texture = &InterpolationContext->Material->Texture.Get(InterpolationContext->MipMappingLevel);
    VLN_ASSERT(Texture);

    TextureBuffer = Texture->GetBuffer();
    TexturePitch = Texture->GetPitch();
    const f32 TextureSize = (f32)Texture->GetWidth();

    for (i32f I = 0; I < 3; ++I)
    {
        UVtx[I] = IntToFx16((i32)(InterpolationContext->Vtx[I].U * TextureSize + 0.5f));
        VVtx[I] = IntToFx16((i32)(InterpolationContext->Vtx[I].V * TextureSize + 0.5f));
    }
}

void VAffineTextureInterpolator::ComputeYStartsAndDeltasLeft(i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx)
{
    ULeft = UVtx[LeftStartVtx];
    VLeft = VVtx[LeftStartVtx];

    UDeltaLeftByY = (UVtx[LeftEndVtx] - UVtx[LeftStartVtx]) / YDiffLeft;
    VDeltaLeftByY = (VVtx[LeftEndVtx] - VVtx[LeftStartVtx]) / YDiffLeft;
}

void VAffineTextureInterpolator::ComputeYStartsAndDeltasRight(i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx)
{
    URight = UVtx[RightStartVtx];
    VRight = VVtx[RightStartVtx];

    UDeltaRightByY = (UVtx[RightEndVtx] - UVtx[RightStartVtx]) / YDiffRight;
    VDeltaRightByY = (VVtx[RightEndVtx] - VVtx[RightStartVtx]) / YDiffRight;
}

void VAffineTextureInterpolator::SwapLeftRight()
{
    i32 TempInt;

    VLN_SWAP(UDeltaLeftByY, UDeltaRightByY, TempInt);
    VLN_SWAP(VDeltaLeftByY, VDeltaRightByY, TempInt);

    VLN_SWAP(ULeft, URight, TempInt);
    VLN_SWAP(VLeft, VRight, TempInt);

    VLN_SWAP(UVtx[InterpolationContext->VtxIndices[1]], UVtx[InterpolationContext->VtxIndices[2]], TempInt);
    VLN_SWAP(VVtx[InterpolationContext->VtxIndices[1]], VVtx[InterpolationContext->VtxIndices[2]], TempInt);
}

void VAffineTextureInterpolator::ComputeXStartsAndDeltas(i32 XDiff, fx28 ZLeft, fx28 ZRight)
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

void VAffineTextureInterpolator::ProcessPixel()
{
    const VColorARGB TextureColor = TextureBuffer[Fx16ToInt(V) * TexturePitch + Fx16ToInt(U)];
    const VColorARGB Pixel = InterpolationContext->Pixel;

    InterpolationContext->Pixel = MAP_XRGB32(
        (TextureColor.R * Pixel.R) >> 8,
        (TextureColor.G * Pixel.G) >> 8,
        (TextureColor.B * Pixel.B) >> 8
    );
}

void VAffineTextureInterpolator::InterpolateX(i32 X)
{
    U += UDeltaByX * X;
    V += VDeltaByX * X;
}

void VAffineTextureInterpolator::InterpolateYLeft(i32 YLeft)
{
    ULeft += UDeltaLeftByY * YLeft;
    VLeft += VDeltaLeftByY * YLeft;
}

void VAffineTextureInterpolator::InterpolateYRight(i32 YRight)
{
    URight += UDeltaRightByY * YRight;
    VRight += VDeltaRightByY * YRight;
}

}
