#include "Engine/Graphics/Scene/Material.h"

namespace Volition
{

void VMaterial::Init()
{
    Attr = 0;
    RAmbient = RDiffuse = RSpecular = Color = VColorARGB::White;
    KAmbient = KDiffuse = KSpecular = Power = 1.0f;
    Texture.Destroy();
}

void VMaterial::Destroy()
{
    Texture.Destroy();
}

}