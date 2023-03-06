#pragma once

#include "Engine/Core/Containers/Array.h"
#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/GameState.h"

class VWorld
{
    TArray<VEntity*> Entities;
    VCamera* Camera;

    VGameState* GameFlow;

public:
    template <typename GameStateT>
    void StartUp()
    {
        Camera = new VCamera();

        GameFlow = new GameStateT();
        GameFlow->StartUp();
    }

    void ShutDown()
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
        T* Entity = Entities.EmplaceBack(new T());
        Entity->Init();
        return Entity;
    }

    friend class VRenderer;
    friend class VRenderContext;
};

extern VWorld World;