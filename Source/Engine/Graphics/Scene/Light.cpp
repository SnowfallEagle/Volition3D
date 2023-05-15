#include "Engine/Graphics/Scene/Light.h"

namespace Volition
{

void VLight::Init(ELightType InType)
{
    bActive = true;
    Type = InType;

    switch (InType)
    {
    case ELightType::Ambient:
    {
        Color = MAP_XRGB32(0x33, 0x22, 0x11);
        Position = { 0.0f, 0.0f, 0.0f };
    } break;

    case ELightType::Infinite:
    {

    } break;

    case ELightType::Point:
    {

    } break;

    case ELightType::SimpleSpotlight:
    {

    } break;

    case ELightType::ComplexSpotlight:
    {

    } break;
    }
}

}