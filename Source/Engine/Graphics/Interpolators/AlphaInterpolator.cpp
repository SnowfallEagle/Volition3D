#include "Engine/Graphics/Rendering/InterpolationContext.h"
#include "Engine/Graphics/Interpolators/AlphaInterpolator.h"

namespace Volition
{

#if 0
void VAlphaInterpolator::Start()
{
    Alpha = InterpolationContext->LitColor[0].A;
}

void VAlphaInterpolator::ProcessPixel()
{
    const VColorARGB Pixel = InterpolationContext->Pixel;
    const VColorARGB BufferPixel = InterpolationContext->Buffer[
        InterpolationContext->Y * InterpolationContext->BufferPitch + InterpolationContext->X
    ];

    InterpolationContext->Pixel = MAP_XRGB32(
        ( (Alpha * Pixel.R) + ((255 - Alpha) * BufferPixel.R) ) >> 8,
        ( (Alpha * Pixel.G) + ((255 - Alpha) * BufferPixel.G) ) >> 8,
        ( (Alpha * Pixel.B) + ((255 - Alpha) * BufferPixel.B) ) >> 8
    );
}
#endif

VAlphaInterpolator::VAlphaInterpolator()
{
}

}
