#pragma once

#include "Common/Types/Array.h"
#include "Engine/World/Terrain.h"
#include "Engine/World/Entity.h"
#include "Engine/World/GameState.h"

namespace Volition
{

class VWorld
{
    VGameState* GameState;

    TArray<VEntity*> Entities;
    VCamera* Camera;

    VSurface Cubemap;
    float CubemapMovementEffectAngle = 0.0f;

    VTerrain* Terrain;

public:
    template<typename GameStateT>
    void StartUp()
    {
        static constexpr i32f MinEntitiesCapacity = 128;

        Entities.Resize(MinEntitiesCapacity);

        for (i32f i = 0; i < MinEntitiesCapacity; ++i)
        {
            Entities[i] = nullptr;
        }

        Camera = new VCamera();

        Terrain = new VTerrain();
        Terrain->Init();
        Terrain->Mesh->Position = { -1000.0f, -425.0f, 1000.0f };

        GameState = new GameStateT();
        GameState->StartUp();
    }

    void ShutDown();
    void Update(f32 DeltaTime);
    void FixedUpdate(f32 FixedDeltaTime);

    VLN_FINLINE VCamera* GetCamera() const
    {
        return Camera;
    }

    VLN_FINLINE VTerrain* GetTerrain() const
    {
        return Terrain;
    }

    void SetCubemap(const char* Path);

    template<typename T = VEntity>
    T* SpawnEntity()
    {
        VEntity* Entity = Entities.EmplaceBack(new T());
        Entity->Init();
        return (T*)Entity;
    }

    void DestroyEntity(VEntity* Entity);

    friend class VRenderer;
};

inline VWorld World;

}