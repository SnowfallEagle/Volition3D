#pragma once

#include "Engine/Core/Containers/Array.h"
#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/GameState.h"

namespace Volition
{

class VWorld
{
    TArray<VEntity*> Entities;
    VCamera* Camera;

    VGameState* GameFlow;

public:
    template<typename GameStateT>
    void StartUp()
    {
        Camera = new VCamera();

        GameFlow = new GameStateT();
        GameFlow->StartUp();
    }

    void ShutDown();
    void Update(f32 DeltaTime);

    VLN_FINLINE VCamera* GetCamera()
    {
        return Camera;
    }

    template<typename T>
    T* SpawnEntity()
    {
        T* Entity = Entities.EmplaceBack(new T());
        Entity->Init();
        return Entity;
    }

    friend class VRenderer;
};

extern VWorld World;

}