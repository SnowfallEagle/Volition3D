#include "Engine/GameCore/Entity.h"

namespace Volition
{

void VEntity::Destroy()
{
    if (Mesh)
    {
        Mesh->Destroy();
        delete Mesh;
        Mesh = nullptr;
    }
}

}
