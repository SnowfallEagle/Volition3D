#include "Engine/Graphics/Rendering/InterpolationContext.h"
#include "Engine/Graphics/Interpolators/EmissiveInterpolator.h"

namespace Volition
{

static void StartFun(VFlatInterpolator* Self)
{
    Self->Color = Self->InterpolationContext->OriginalColor;
}

static void ProcessPixelFun(VFlatInterpolator* Self)
{
    const VColorARGB Pixel = Self->InterpolationContext->Pixel;

    Self->InterpolationContext->Pixel = MAP_XRGB32(
        (Self->Color.R * Pixel.R) >> 8,
        (Self->Color.G * Pixel.G) >> 8,
        (Self->Color.B * Pixel.B) >> 8
    );
}

VEmissiveInterpolator::VEmissiveInterpolator()
{
    Start = (StartType)StartFun;
    ProcessPixel = (ProcessPixelType)ProcessPixelFun;
}

}
