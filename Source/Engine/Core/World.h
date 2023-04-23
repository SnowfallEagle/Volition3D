#pragma once

#include "Engine/Core/Containers/Array.h"
#include "Engine/GameCore/Entity.h"
#include "Engine/GameCore/GameState.h"

namespace Volition
{

class VWorld
{
    TArray<VEntity*> Entities;
    VCamera* Camera;
    VEntity* Cubemap = nullptr;

    VGameState* GameState;

public:
    template<typename GameStateT>
    void StartUp()
    {
        Camera = new VCamera();

        GameState = new GameStateT();
        GameState->StartUp();
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

    void SetCubemap(const char* Path);

    friend class VRenderer;
};

extern VWorld World;

}