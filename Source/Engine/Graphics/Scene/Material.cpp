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

}