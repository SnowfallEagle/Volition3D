#include "Engine/Graphics/Rendering/InterpolationContext.h"
#include "Engine/Graphics/Interpolators/BillinearPerspectiveTextureInterpolator.h"

namespace Volition
{

void VBillinearPerspectiveTextureInterpolator::Start()
{
    const VSurface* Texture = &InterpolationContext->Material->Texture.Get(InterpolationContext->MipMappingLevel);
    VLN_ASSERT(Texture);

    TextureBuffer = Texture->GetBuffer();
    TexturePitch = Texture->GetPitch();
    TextureSize = Texture->GetWidth();

    for (i32f I = 0; I < 3; ++I)
    {
        UVtx[I] =
            IntToFx22((i32)(InterpolationContext->Vtx[I].U * (f32)TextureSize + 0.5f)) /
            (i32)(InterpolationContext->Vtx[I].Z + 0.5f);
        VVtx[I] =
            IntToFx22((i32)(InterpolationContext->Vtx[I].V * (f32)TextureSize + 0.5f)) /
            (i32)(InterpolationContext->Vtx[I].Z + 0.5f);
    }
}

void VBillinearPerspectiveTextureInterpolator::ComputeYStartsAndDeltasLeft(i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx)
{
    ULeft = (UVtx[LeftStartVtx]);
    VLeft = (VVtx[LeftStartVtx]);

    UDeltaLeftByY = (UVtx[LeftEndVtx] - UVtx[LeftStartVtx]) / YDiffLeft;
    VDeltaLeftByY = (VVtx[LeftEndVtx] - VVtx[LeftStartVtx]) / YDiffLeft;
}

void VBillinearPerspectiveTextureInterpolator::ComputeYStartsAndDeltasRight(i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx)
{
    URight = (UVtx[RightStartVtx]);
    VRight = (VVtx[RightStartVtx]);

    UDeltaRightByY = (UVtx[RightEndVtx] - UVtx[RightStartVtx]) / YDiffRight;
    VDeltaRightByY = (VVtx[RightEndVtx] - VVtx[RightStartVtx]) / YDiffRight;
}

void VBillinearPerspectiveTextureInterpolator::SwapLeftRight()
{
    i32 TempInt;

    VLN_SWAP(UDeltaLeftByY, UDeltaRightByY, TempInt);
    VLN_SWAP(VDeltaLeftByY, VDeltaRightByY, TempInt);

    VLN_SWAP(ULeft, URight, TempInt);
    VLN_SWAP(VLeft, VRight, TempInt);

    VLN_SWAP(UVtx[InterpolationContext->VtxIndices[1]], UVtx[InterpolationContext->VtxIndices[2]], TempInt);
    VLN_SWAP(VVtx[InterpolationContext->VtxIndices[1]], VVtx[InterpolationContext->VtxIndices[2]], TempInt);
}

void VBillinearPerspectiveTextureInterpolator::ComputeXStartsAndDeltas(i32 XDiff, fx28 ZLeft, fx28 ZRight)
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

void VBillinearPerspectiveTextureInterpolator::ProcessPixel()
{
    const i32f X0 = ((U << (Fx28Shift - Fx22Shift)) / InterpolationContext->Z);
    const i32f Y0 = ((V << (Fx28Shift - Fx22Shift)) / InterpolationContext->Z) * TexturePitch;

    i32f X1 = X0 + 1;
    if (X1 >= TextureSize)
    {
        X1 = X0;
    }

    i32f Y1 = Y0 + TexturePitch;
    if (Y1 >= (TextureSize * TexturePitch))
    {
        Y1 = Y0;
    }

    const VColorARGB TextureColors[4] = {
        TextureBuffer[Y0 + X0],
        TextureBuffer[Y0 + X1],
        TextureBuffer[Y1 + X0],
        TextureBuffer[Y1 + X1],
    };

    // (fx22 -> fx8) & 0xFF
    const i32 FracU = (U >> 14) & 0xFF;
    const i32 FracV = (V >> 14) & 0xFF;

    // fx8<1> - fx8<Frac>
    const i32 OneMinusFracU = (1 << 8) - FracU;
    const i32 OneMinusFracV = (1 << 8) - FracV;

    // fx8 * fx8 = fx16
    const i32 OneMinusFracUMulOneMinusFracV = OneMinusFracU * OneMinusFracV;
    const i32 FracUMulOneMinusFracV         = FracU         * OneMinusFracV;
    const i32 OneMinusFracUMulFracV         = OneMinusFracU * FracV;
    const i32 FracUMulFracV                 = FracU         * FracV;

    // fx16 * fx8 = fx24 -> fx8
    VColorARGB FilteredColor;
    FilteredColor.R = (
        OneMinusFracUMulOneMinusFracV * TextureColors[0].R +
        FracUMulOneMinusFracV         * TextureColors[1].R +
        OneMinusFracUMulFracV         * TextureColors[2].R +
        FracUMulFracV                 * TextureColors[3].R
    ) >> 16;

    FilteredColor.G = (
        OneMinusFracUMulOneMinusFracV * TextureColors[0].G +
        FracUMulOneMinusFracV         * TextureColors[1].G +
        OneMinusFracUMulFracV         * TextureColors[2].G +
        FracUMulFracV                 * TextureColors[3].G
    ) >> 16;

    FilteredColor.B = (
        OneMinusFracUMulOneMinusFracV * TextureColors[0].B +
        FracUMulOneMinusFracV         * TextureColors[1].B +
        OneMinusFracUMulFracV         * TextureColors[2].B +
        FracUMulFracV                 * TextureColors[3].B
    ) >> 16;

    const VColorARGB Pixel = InterpolationContext->Pixel;

    InterpolationContext->Pixel = MAP_XRGB32(
        (FilteredColor.R * Pixel.R) >> 8,
        (FilteredColor.G * Pixel.G) >> 8,
        (FilteredColor.B * Pixel.B) >> 8
    );
}

void VBillinearPerspectiveTextureInterpolator::InterpolateX(i32 X)
{
    U += UDeltaByX * X;
    V += VDeltaByX * X;
}

void VBillinearPerspectiveTextureInterpolator::InterpolateYLeft(i32 YLeft)
{
    ULeft += UDeltaLeftByY * YLeft;
    VLeft += VDeltaLeftByY * YLeft;
}

void VBillinearPerspectiveTextureInterpolator::InterpolateYRight(i32 YRight)
{
    URight += UDeltaRightByY * YRight;
    VRight += VDeltaRightByY * YRight;
}

}