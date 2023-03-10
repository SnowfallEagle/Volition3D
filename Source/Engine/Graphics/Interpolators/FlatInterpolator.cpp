#include "Engine/Graphics/InterpolationContext.h"
#include "Engine/Graphics/Interpolators/FlatInterpolator.h"

namespace Volition
{

void VFlatInterpolator::Start()
{
    Color = InterpolationContext->LitColor[0];
}

void VFlatInterpolator::ProcessPixel()
{
    VColorARGB Pixel = InterpolationContext->Pixel;

    InterpolationContext->Pixel = MAP_XRGB32(
        (Color.R * Pixel.R) >> 8,
        (Color.G * Pixel.G) >> 8,
        (Color.B * Pixel.B) >> 8
    );
}

}
