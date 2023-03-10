#include "Engine/Core/World.h"

namespace Volition
{

VWorld World;

void VWorld::ShutDown()
{
    if (GameFlow)
    {
        GameFlow->ShutDown();
        delete GameFlow;
    }

    for (auto Entity : Entities)
    {
        if (Entity)
        {
            Entity->Destroy();
            delete Entity;
            Entity = nullptr;
        }
    }

    if (Camera)
    {
        delete Camera;
        Camera = nullptr;
    }
}

void VWorld::Update(f32 DeltaTime)
{
    if (GameFlow)
    {
        GameFlow->Update(DeltaTime);
    }

    for (auto Entity : Entities)
    {
        if (Entity)
        {
            Entity->Update(DeltaTime);
        }
    }
}

}