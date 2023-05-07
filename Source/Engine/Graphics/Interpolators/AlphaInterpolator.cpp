#include "Engine/Graphics/Rendering/InterpolationContext.h"
#include "Engine/Graphics/Interpolators/AlphaInterpolator.h"

namespace Volition
{

static void StartFun(VAlphaInterpolator* Self)
{
    Self->Alpha = Self->InterpolationContext->LitColor[0].A;
}

static void ProcessPixelFun(VAlphaInterpolator* Self)
{
    const VColorARGB Pixel = Self->InterpolationContext->Pixel;
    const VColorARGB BufferPixel = Self->InterpolationContext->Buffer[
        Self->InterpolationContext->Y * Self->InterpolationContext->BufferPitch + Self->InterpolationContext->X
    ];

    Self->InterpolationContext->Pixel = MAP_XRGB32(
        ( (Self->Alpha * Pixel.R) + ((255 - Self->Alpha) * BufferPixel.R) ) >> 8,
        ( (Self->Alpha * Pixel.G) + ((255 - Self->Alpha) * BufferPixel.G) ) >> 8,
        ( (Self->Alpha * Pixel.B) + ((255 - Self->Alpha) * BufferPixel.B) ) >> 8
    );
}

VAlphaInterpolator::VAlphaInterpolator()
{
    Start = (StartType)StartFun;
    ProcessPixel = (ProcessPixelType)ProcessPixelFun;
}

}
