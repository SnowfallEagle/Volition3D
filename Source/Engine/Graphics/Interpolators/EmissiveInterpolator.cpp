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
    const VColorARGB Color = Self->Color;

    Self->InterpolationContext->Pixel = MAP_XRGB32(
        (Color.R * Pixel.R) >> 8,
        (Color.G * Pixel.G) >> 8,
        (Color.B * Pixel.B) >> 8
    );
}

VEmissiveInterpolator::VEmissiveInterpolator()
{
    Start = (StartType)StartFun;
    ProcessPixel = (ProcessPixelType)ProcessPixelFun;
}

}
