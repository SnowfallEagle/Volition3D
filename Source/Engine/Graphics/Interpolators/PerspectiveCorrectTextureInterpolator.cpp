
#include "Engine/Graphics/InterpolationContext.h"
#include "Engine/Graphics/Interpolators/PerspectiveCorrectTextureInterpolator.h"

namespace Volition
{

void VPerspectiveCorrectTextureInterpolator::Start()
{
    const VSurface* Texture = &InterpolationContext->Material->Texture.Get(InterpolationContext->MipMappingLevel);
    VLN_ASSERT(Texture);

    TextureBuffer = Texture->GetBuffer();
    TexturePitch = Texture->GetPitch();
    const f32 TextureSize = (f32)Texture->GetWidth();

    for (i32f I = 0; I < 3; ++I)
    {
        UVtx[I] =
            IntToFx22((i32)(InterpolationContext->Vtx[I].U * TextureSize + 0.5f)) /
                (i32)(InterpolationContext->Vtx[I].Z + 0.5f);
        VVtx[I] =
            IntToFx22((i32)(InterpolationContext->Vtx[I].V * TextureSize + 0.5f)) /
                (i32)(InterpolationContext->Vtx[I].Z + 0.5f);
    }
}

void VPerspectiveCorrectTextureInterpolator::ComputeYStartsAndDeltasLeft(i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx)
{
    ULeft = (UVtx[LeftStartVtx]);
    VLeft = (VVtx[LeftStartVtx]);

    UDeltaLeftByY = (UVtx[LeftEndVtx] - UVtx[LeftStartVtx]) / YDiffLeft;
    VDeltaLeftByY = (VVtx[LeftEndVtx] - VVtx[LeftStartVtx]) / YDiffLeft;
}

void VPerspectiveCorrectTextureInterpolator::ComputeYStartsAndDeltasRight(i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx)
{
    URight = (UVtx[RightStartVtx]);
    VRight = (VVtx[RightStartVtx]);

    UDeltaRightByY = (UVtx[RightEndVtx] - UVtx[RightStartVtx]) / YDiffRight;
    VDeltaRightByY = (VVtx[RightEndVtx] - VVtx[RightStartVtx]) / YDiffRight;
}

void VPerspectiveCorrectTextureInterpolator::SwapLeftRight()
{
    i32 TempInt;

    VLN_SWAP(UDeltaLeftByY, UDeltaRightByY, TempInt);
    VLN_SWAP(VDeltaLeftByY, VDeltaRightByY, TempInt);

    VLN_SWAP(ULeft, URight, TempInt);
    VLN_SWAP(VLeft, VRight, TempInt);

    VLN_SWAP(UVtx[InterpolationContext->VtxIndices[1]], UVtx[InterpolationContext->VtxIndices[2]], TempInt);
    VLN_SWAP(VVtx[InterpolationContext->VtxIndices[1]], VVtx[InterpolationContext->VtxIndices[2]], TempInt);
}

void VPerspectiveCorrectTextureInterpolator::ComputeXStartsAndDeltas(i32 XDiff, fx28 ZLeft, fx28 ZRight)
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

void VPerspectiveCorrectTextureInterpolator::ProcessPixel()
{
    const VColorARGB Pixel = InterpolationContext->Pixel;
    const VColorARGB TextureColor = TextureBuffer[
        ((V << (Fx28Shift - Fx22Shift)) / InterpolationContext->Z) * TexturePitch +
        ((U << (Fx28Shift - Fx22Shift)) / InterpolationContext->Z)
    ];

    InterpolationContext->Pixel = MAP_XRGB32(
        (TextureColor.R * Pixel.R) >> 8,
        (TextureColor.G * Pixel.G) >> 8,
        (TextureColor.B * Pixel.B) >> 8
    );
}

void VPerspectiveCorrectTextureInterpolator::InterpolateX(i32 X)
{
    U += UDeltaByX * X;
    V += VDeltaByX * X;
}

void VPerspectiveCorrectTextureInterpolator::InterpolateYLeft(i32 YLeft)
{
    ULeft += UDeltaLeftByY * YLeft;
    VLeft += VDeltaLeftByY * YLeft;
}

void VPerspectiveCorrectTextureInterpolator::InterpolateYRight(i32 YRight)
{
    URight += UDeltaRightByY * YRight;
    VRight += VDeltaRightByY * YRight;
}

}
