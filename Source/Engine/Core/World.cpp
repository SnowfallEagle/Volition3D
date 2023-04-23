#include "Engine/Graphics/Renderer.h"
#include "Engine/Core/World.h"

namespace Volition
{

VWorld World;

void VWorld::ShutDown()
{
    if (GameState)
    {
        GameState->ShutDown();
        delete GameState;
    }

    for (const auto Entity : Entities)
    {
        if (Entity)
        {
            Entity->Destroy();
            delete Entity;
        }
    }
    Entities.Clear();

    VLN_SAFE_DELETE(Camera);
    VLN_SAFE_DELETE(Cubemap);
}

void VWorld::Update(f32 DeltaTime)
{
    if (GameState)
    {
        GameState->Update(DeltaTime);
    }

    for (const auto Entity : Entities)
    {
        if (Entity)
        {
            Entity->Update(DeltaTime);
        }
    }
}

void VWorld::SetCubemap(const char* Path)
{
    const auto Entity = SpawnEntity<VEntity>();
    Entity->Mesh->LoadCubemap(Path);
}

}