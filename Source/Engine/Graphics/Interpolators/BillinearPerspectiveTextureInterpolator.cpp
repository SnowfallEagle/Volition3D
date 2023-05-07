#include "Engine/Graphics/Rendering/InterpolationContext.h"
#include "Engine/Graphics/Interpolators/BillinearPerspectiveTextureInterpolator.h"

namespace Volition
{

static void StartFun(VBillinearPerspectiveTextureInterpolator* Self)
{
    const VSurface* Texture = &Self->InterpolationContext->Material->Texture.Get(Self->InterpolationContext->MipMappingLevel);
    VLN_ASSERT(Texture);

    Self->TextureBuffer = Texture->GetBuffer();
    Self->TexturePitch = Texture->GetPitch();
    Self->TextureSize = Texture->GetWidth();

    for (i32f i = 0; i < 3; ++i)
    {
        Self->UVtx[i] =
            IntToFx22((i32)(Self->InterpolationContext->Vtx[i].U * (f32)Self->TextureSize + 0.5f)) /
            (i32)(Self->InterpolationContext->Vtx[i].Z + 0.5f);
        Self->VVtx[i] =
            IntToFx22((i32)(Self->InterpolationContext->Vtx[i].V * (f32)Self->TextureSize + 0.5f)) /
            (i32)(Self->InterpolationContext->Vtx[i].Z + 0.5f);
    }
}

static void ComputeYStartsAndDeltasLeftFun(VBillinearPerspectiveTextureInterpolator* Self, i32 YDiffLeft, i32 LeftStartVtx, i32 LeftEndVtx)
{
    Self->ULeft = (Self->UVtx[LeftStartVtx]);
    Self->VLeft = (Self->VVtx[LeftStartVtx]);

    Self->UDeltaLeftByY = (Self->UVtx[LeftEndVtx] - Self->UVtx[LeftStartVtx]) / YDiffLeft;
    Self->VDeltaLeftByY = (Self->VVtx[LeftEndVtx] - Self->VVtx[LeftStartVtx]) / YDiffLeft;
}

static void ComputeYStartsAndDeltasRightFun(VBillinearPerspectiveTextureInterpolator* Self, i32 YDiffRight, i32 RightStartVtx, i32 RightEndVtx)
{
    Self->URight = (Self->UVtx[RightStartVtx]);
    Self->VRight = (Self->VVtx[RightStartVtx]);

    Self->UDeltaRightByY = (Self->UVtx[RightEndVtx] - Self->UVtx[RightStartVtx]) / YDiffRight;
    Self->VDeltaRightByY = (Self->VVtx[RightEndVtx] - Self->VVtx[RightStartVtx]) / YDiffRight;
}

static void SwapLeftRightFun(VBillinearPerspectiveTextureInterpolator* Self)
{
    i32 TempInt;

    VLN_SWAP(Self->UDeltaLeftByY, Self->UDeltaRightByY, TempInt);
    VLN_SWAP(Self->VDeltaLeftByY, Self->VDeltaRightByY, TempInt);

    VLN_SWAP(Self->ULeft, Self->URight, TempInt);
    VLN_SWAP(Self->VLeft, Self->VRight, TempInt);

    VLN_SWAP(Self->UVtx[Self->InterpolationContext->VtxIndices[1]], Self->UVtx[Self->InterpolationContext->VtxIndices[2]], TempInt);
    VLN_SWAP(Self->VVtx[Self->InterpolationContext->VtxIndices[1]], Self->VVtx[Self->InterpolationContext->VtxIndices[2]], TempInt);
}

static void ComputeXStartsAndDeltasFun(VBillinearPerspectiveTextureInterpolator* Self, i32 XDiff, fx28 ZLeft, fx28 ZRight)
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

static void ProcessPixelFun(VBillinearPerspectiveTextureInterpolator* Self)
{
    const i32f X0 = ((Self->U << (Fx28Shift - Fx22Shift)) / Self->InterpolationContext->Z);
    const i32f Y0 = ((Self->V << (Fx28Shift - Fx22Shift)) / Self->InterpolationContext->Z) * Self->TexturePitch;

    i32f X1 = X0 + 1;
    if (X1 >= Self->TextureSize)
    {
        X1 = X0;
    }

    i32f Y1 = Y0 + Self->TexturePitch;
    if (Y1 >= (Self->TextureSize * Self->TexturePitch))
    {
        Y1 = Y0;
    }

    const VColorARGB TextureColors[4] = {
        Self->TextureBuffer[Y0 + X0],
        Self->TextureBuffer[Y0 + X1],
        Self->TextureBuffer[Y1 + X0],
        Self->TextureBuffer[Y1 + X1],
    };

    // (fx22 -> fx8) & 0xFF
    const i32 FracU = (Self->U >> 14) & 0xFF;
    const i32 FracV = (Self->V >> 14) & 0xFF;

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

    const VColorARGB Pixel = Self->InterpolationContext->Pixel;

    Self->InterpolationContext->Pixel = MAP_XRGB32(
        (FilteredColor.R * Pixel.R) >> 8,
        (FilteredColor.G * Pixel.G) >> 8,
        (FilteredColor.B * Pixel.B) >> 8
    );
}

static void InterpolateXFun(VBillinearPerspectiveTextureInterpolator* Self, i32 X)
{
    Self->U += Self->UDeltaByX * X;
    Self->V += Self->VDeltaByX * X;
}

static void InterpolateYLeftFun(VBillinearPerspectiveTextureInterpolator* Self, i32 YLeft)
{
    Self->ULeft += Self->UDeltaLeftByY * YLeft;
    Self->VLeft += Self->VDeltaLeftByY * YLeft;
}

static void InterpolateYRightFun(VBillinearPerspectiveTextureInterpolator* Self, i32 YRight)
{
    Self->URight += Self->UDeltaRightByY * YRight;
    Self->VRight += Self->VDeltaRightByY * YRight;
}

VBillinearPerspectiveTextureInterpolator::VBillinearPerspectiveTextureInterpolator()
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