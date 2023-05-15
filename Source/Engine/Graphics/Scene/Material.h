#pragma once

#include "Common/Platform/Platform.h"
#include "Common/Types/Common.h"
#include "Engine/Graphics/Types/Color.h"
#include "Engine/Graphics/Rendering/Texture.h"

namespace Volition
{

class VMaterial
{
public:
    u32 Attr; // @TODO: EPolyAttr

    VColorARGB Color;
    f32 KAmbient, KDiffuse, KSpecular, Power;
    // @TODO: Use it in lighting
    // @TODO: Do we check overflow?
    VColorARGB RAmbient, RDiffuse, RSpecular; // K * Color

    VTexture Texture;

public:
    void Init();
    void Destroy();
};

}
