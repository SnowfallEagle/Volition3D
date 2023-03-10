#include "Engine/GameFramework/Entity.h"

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
