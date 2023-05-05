#include "Engine/Graphics/Rendering/InterpolationContext.h"
#include "Engine/Graphics/Interpolators/LinearPiecewiseTextureInterpolator.h"

namespace Volition
{

void VLinearPiecewiseTextureInterpolator::Start()
{
    const VSurface* Texture = &InterpolationContext->Material->Texture.Get(InterpolationContext->MipMappingLevel);
    VLN_ASSERT(Texture);

    TextureBuffer = Texture->GetBuffer();
    TexturePitch = Texture->GetPitch();
    const f32 TextureSize = (f32)Texture->GetWidth();

    for (i32f i = 0; i < 3; ++i)
    {
        UVtx[i] =
            IntToFx22((i32)(InterpolationContext->Vtx[i].U * TextureSize + 0.5f)) /
                (i32)(InterpolationContext->Vtx[i].Z + 0.5f);
        VVtx[i] =
            IntToFx22((i32)(InterpolationContext->Vtx[i].V * TextureSize + 0.5f)) /
                (i32)(InterpolationContext->Vtx[i].Z + 0.5f);
    }
}

void VLinearPiecewiseTextureInterpolator::ComputeYStartsAndDeltasLeft(i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx)
{
    ULeft = (UVtx[LeftStartVtx]);
    VLeft = (VVtx[LeftStartVtx]);

    UDeltaLeftByY = (UVtx[LeftEndVtx] - UVtx[LeftStartVtx]) / YDiffLeft;
    VDeltaLeftByY = (VVtx[LeftEndVtx] - VVtx[LeftStartVtx]) / YDiffLeft;
}

void VLinearPiecewiseTextureInterpolator::ComputeYStartsAndDeltasRight(i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx)
{
    URight = (UVtx[RightStartVtx]);
    VRight = (VVtx[RightStartVtx]);

    UDeltaRightByY = (UVtx[RightEndVtx] - UVtx[RightStartVtx]) / YDiffRight;
    VDeltaRightByY = (VVtx[RightEndVtx] - VVtx[RightStartVtx]) / YDiffRight;
}

void VLinearPiecewiseTextureInterpolator::SwapLeftRight()
{
    i32 TempInt;

    VLN_SWAP(UDeltaLeftByY, UDeltaRightByY, TempInt);
    VLN_SWAP(VDeltaLeftByY, VDeltaRightByY, TempInt);

    VLN_SWAP(ULeft, URight, TempInt);
    VLN_SWAP(VLeft, VRight, TempInt);

    VLN_SWAP(UVtx[InterpolationContext->VtxIndices[1]], UVtx[InterpolationContext->VtxIndices[2]], TempInt);
    VLN_SWAP(VVtx[InterpolationContext->VtxIndices[1]], VVtx[InterpolationContext->VtxIndices[2]], TempInt);
}

void VLinearPiecewiseTextureInterpolator::ComputeXStartsAndDeltas(i32 XDiff, fx28 ZLeft, fx28 ZRight)
{
    const fx22 ULeftDivZ = ((ULeft << (Fx28Shift - Fx22Shift)) / (ZLeft >> 6)) << 16;
    const fx22 URightDivZ = ((URight << (Fx28Shift - Fx22Shift)) / (ZRight >> 6)) << 16;

    const fx22 VLeftDivZ = ((VLeft << (Fx28Shift - Fx22Shift)) / (ZLeft >> 6)) << 16;
    const fx22 VRightDivZ = ((VRight << (Fx28Shift - Fx22Shift)) / (ZRight >> 6)) << 16;

    U = ULeftDivZ;
    V = VLeftDivZ;

    if (XDiff > 0)
    {
        UDeltaByX = (URightDivZ - ULeftDivZ) / XDiff;
        VDeltaByX = (VRightDivZ - VLeftDivZ) / XDiff;
    }
    else
    {
        UDeltaByX = (URightDivZ - ULeftDivZ);
        VDeltaByX = (VRightDivZ - VLeftDivZ);
    }
}

void VLinearPiecewiseTextureInterpolator::InterpolateX(i32 X)
{
    U += UDeltaByX * X;
    V += VDeltaByX * X;
}

void VLinearPiecewiseTextureInterpolator::InterpolateYLeft(i32 YLeft)
{
    ULeft += UDeltaLeftByY * YLeft;
    VLeft += VDeltaLeftByY * YLeft;
}

void VLinearPiecewiseTextureInterpolator::InterpolateYRight(i32 YRight)
{
    URight += UDeltaRightByY * YRight;
    VRight += VDeltaRightByY * YRight;
}

void VLinearPiecewiseTextureInterpolator::ProcessPixel()
{
    const VColorARGB Pixel = InterpolationContext->Pixel;
    const VColorARGB TextureColor = TextureBuffer[Fx22ToInt(V) * TexturePitch + Fx22ToInt(U)];

    InterpolationContext->Pixel = MAP_XRGB32(
        (TextureColor.R * Pixel.R) >> 8,
        (TextureColor.G * Pixel.G) >> 8,
        (TextureColor.B * Pixel.B) >> 8
    );
}

}

