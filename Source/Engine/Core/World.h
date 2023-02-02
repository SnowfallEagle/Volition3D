#pragma once

#include "Engine/Core/Containers/SparseArray.h"
#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/GameFlow.h"

class VWorld
{
    TSparseArray<VEntity*> Entities;
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
        if (GameFlow)
        {
            GameFlow->ShutDown();
            delete GameFlow;
        }

        for (auto& Entity : Entities)
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

    void Update(f32 DeltaTime)
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

    VL_FINLINE VCamera* GetCamera()
    {
        return Camera;
    }

    template <typename T>
    T* SpawnEntity()
    {
        VEntity* Entity = new T();
        Entity->Init();

        Entities.EmplaceBack(Entity);
        return Entity;
    }

    friend class VRenderer;
    friend class VRenderContext;
};

extern VWorld World;