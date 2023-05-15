#pragma once

#include "Common/Types/Array.h"
#include "Engine/Graphics/Scene/Light.h"
#include "Engine/World/Terrain.h"
#include "Engine/World/Entity.h"
#include "Engine/World/GameState.h"

namespace Volition
{

class VWorld
{
private:
    static constexpr i32f MinEntitiesCapacity = 128;
    static constexpr i32f MinLightsCapacity = 128;

private:
    VGameState* GameState;

    TArray<VEntity*> Entities;
    TArray<VLight> Lights;
    VLight* OccluderLight;

    VSurface Cubemap;
    f32 CubemapMovementEffectAngle = 0.0f;

    VCamera* Camera;
    VTerrain* Terrain;

public:
    template<typename GameStateT>
    void StartUp();
    void ShutDown();

    void Update(f32 DeltaTime);
    void FixedUpdate(f32 FixedDeltaTime);

    void SetCubemap(const char* Path);

    template<typename T = VEntity>
    T* SpawnEntity();
    void DestroyEntity(VEntity* Entity);

    VLight* SpawnLight(ELightType Type);

    VLN_FINLINE VCamera* GetCamera() const
    {
        return Camera;
    }

    VLN_FINLINE VTerrain* GetTerrain() const
    {
        return Terrain;
    }

    VLN_FINLINE void SetOccluderLight(VLight* Light)
    {
        OccluderLight = Light;
    }

    friend class VRenderer;
};

inline VWorld World;

template<typename GameStateT>
void VWorld::StartUp()
{
    Entities.Resize(MinEntitiesCapacity);

    Lights.Resize(MinLightsCapacity);
    OccluderLight = nullptr;

    Camera = new VCamera();
    Camera->Init(ECameraAttr::Euler, { 0.0f, 1000.0f, 1500.0f }, { 25.0f, 180.0f, 0.0f }, VVector4(), 90.0f, 100.0f, 1000000.0f);

    Terrain = new VTerrain();

    GameState = new GameStateT();
    GameState->StartUp();
}

template<typename T>
T* VWorld::SpawnEntity()
{
    VEntity* Entity = Entities.EmplaceBack(new T());
    Entity->Init();
    return (T*)Entity;
}

}