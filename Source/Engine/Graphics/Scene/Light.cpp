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
    } break;

    case ELightType::Infinite:
    {
        Color = MAP_XRGB32(0x99, 0x66, 0x22);
        Position = { 7500.0f, 7500.0f, 7500.0f }; // For shadows
        Direction = VVector4(-1.0f, -1.0f, 0.0f).GetNormalized();
    } break;

    case ELightType::Point:
    {
        Color = MAP_XRGB32(0xBB, 0x00, 0x10);
        Position = { 0.0f, 0.0f, 0.0f };

        KConst  = 0.0f;
        KLinear = 1.0f;
        KQuad   = 0.0f;
    } break;

    case ELightType::SimpleSpotlight:
    {
        Color = MAP_XRGB32(0x10, 0x00, 0xBB);
        Position = { 0.0f, 0.0f, 0.0f };
        Direction = { 0.0f, -1.0f, 0.0f };

        KConst  = 0.0f;
        KLinear = 1.0f;
        KQuad   = 0.0f;
    } break;

    case ELightType::ComplexSpotlight:
    {
        Color = MAP_XRGB32(0xAA, 0x10, 0xBB);
        Position = { 0.0f, 0.0f, 0.0f };
        Direction = { 0.0f, -1.0f, 0.0f };

        KConst  = 0.0f;
        KLinear = 1.0f;
        KQuad   = 0.0f;

        FalloffPower = 1.0f;
    } break;
    }

}

}