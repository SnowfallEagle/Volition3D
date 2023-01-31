#pragma once

#include "Engine/Core/Containers/SparseArray.h"

class Entity
{
    // TODO(sean)
};

class VWorld
{
    TSparseArray<Entity*> Entities;

public:
    void StartUp()
    {}

    void ShutDown()
    {}

    void Update(f32 DeltaTime)
    {
        /* TODO(sean):
            Update game flow -> update game state
            Update entities
        */
    }

    friend class VRenderer;
};

extern VWorld World;