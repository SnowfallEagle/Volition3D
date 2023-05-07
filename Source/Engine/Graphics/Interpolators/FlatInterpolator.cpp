#include "Engine/Graphics/Rendering/InterpolationContext.h"
#include "Engine/Graphics/Interpolators/FlatInterpolator.h"

namespace Volition
{

static void StartFun(VFlatInterpolator* Self)
{
    Self->Color = Self->InterpolationContext->LitColor[0];
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

VFlatInterpolator::VFlatInterpolator()
{
    Start = (StartType)StartFun;
    ProcessPixel = (ProcessPixelType)ProcessPixelFun;
}

}
