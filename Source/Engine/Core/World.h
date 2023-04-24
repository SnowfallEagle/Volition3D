#pragma once

#include "Engine/Core/Containers/Array.h"
#include "Engine/GameCore/Entity.h"
#include "Engine/GameCore/GameState.h"

namespace Volition
{

class VWorld
{
    VGameState* GameState;

    TArray<VEntity*> Entities;
    VCamera* Camera;

    VSurface Cubemap;
    float CubemapMovementEffectAngle = 0.0f;

public:
    template<typename GameStateT>
    void StartUp()
    {
        static constexpr i32f MinEntitiesCapacity = 128;

        Entities.Resize(MinEntitiesCapacity);

        for (i32f I = 0; I < MinEntitiesCapacity; ++I)
        {
            Entities[I] = nullptr;
        }

        Camera = new VCamera();

        GameState = new GameStateT();
        GameState->StartUp();
    }

    void ShutDown();
    void Update(f32 DeltaTime);
    void FixedUpdate(f32 FixedDeltaTime);

    VLN_FINLINE VCamera* GetCamera()
    {
        return Camera;
    }

    template<typename T = VEntity>
    T* SpawnEntity()
    {
        T* Entity = Entities.EmplaceBack(new T());
        Entity->Init();
        return Entity;
    }

    void DestroyEntity(VEntity* Entity);
    void SetCubemap(const char* Path);

    friend class VRenderer;
};

extern VWorld World;

}