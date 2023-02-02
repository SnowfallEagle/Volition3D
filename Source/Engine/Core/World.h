#pragma once

#include "Engine/Core/Containers/SparseArray.h"
#include "Engine/Graphics/Object.h" // TODO(sean): Entity
#include "Engine/GameFramework/GameFlow.h"

class VEntity
{
    // TODO(sean)
};

class VWorld
{
    TSparseArray<VObject*> Objects;
    VCamera* Camera;

    VGameFlow* GameFlow;

public:
    template <typename GAME_FLOW_TYPE>
    void StartUp()
    {
        Camera = new VCamera();

        GameFlow = new GAME_FLOW_TYPE();
        GameFlow->StartUp();
    }

    void ShutDown()
    {
        GameFlow->ShutDown();
        delete GameFlow;

        // TODO(sean): Delete entities
        for (auto Object : Objects)
        {
            Object->Destroy();
            delete Object;
        }

        delete Camera;
    }

    void Update(f32 DeltaTime)
    {
        GameFlow->Update(DeltaTime);

        // TODO(sean): Update entities
    }

    VL_FINLINE VCamera* GetCamera()
    {
        return Camera;
    }

    template <typename T>
    T* SpawnObject()
    {
        VObject* Object = new T();
        // TODO(sean): Init
        Objects.EmplaceBack(Object);
        return Object;
    }

    friend class VRenderer;
};

extern VWorld World;