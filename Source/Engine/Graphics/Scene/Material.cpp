#include "Engine/Graphics/Scene/Material.h"

namespace Volition
{

void VMaterial::Init()
{
    Attr = 0;
    RAmbient = RDiffuse = Color = VColorARGB::White;
    KAmbient = KDiffuse = Power = 1.0f;
    Texture.Destroy();
}

void VMaterial::Destroy()
{
    Texture.Destroy();
}

void VMaterial::ComputeReflectiveColors()
{
    for (i32f RGBIndex = 1; RGBIndex < 4; ++RGBIndex)
    {
        u32 ColorComponent = (u32)(KAmbient * (f32)Color.C[RGBIndex] + 0.5f);
        if (ColorComponent > 255)
        {
            ColorComponent = 255;
        }
        else if (ColorComponent < 0)
        {
            ColorComponent = 0;
        }
        RAmbient.C[RGBIndex] = ColorComponent;

        ColorComponent = (u32)(KDiffuse * (f32)Color.C[RGBIndex] + 0.5f);
        if (ColorComponent > 255)
        {
            ColorComponent = 255;
        }
        else if (ColorComponent < 0)
        {
            ColorComponent = 0;
        }
        RDiffuse.C[RGBIndex] = ColorComponent;
    }
}

}