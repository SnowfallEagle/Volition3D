#pragma once

#include "Common/Types/Array.h"
#include "Engine/Graphics/Scene/Light.h"
#include "Engine/Graphics/Scene/Material.h"
#include "Engine/World/Entity.h"
#include "Engine/World/GameState.h"

namespace Volition
{

enum class EWorldShutDownReason
{
    Final,
    Reset
};

class VWorld
{
private:
    static constexpr i32f MinEntitiesCapacity  = 128;
    static constexpr i32f MinMaterialsCapacity = 128;
    static constexpr i32f MinLightsCapacity    = 128;

public:
    f32 Environment2DMovementEffectAngle;
    f32 Environment2DMovementEffectSpeed;

private:
    VGameState* GameState;
    VGameState* NextGameState;

    TArray<VEntity*> Entities;
    TArray<VMaterial> Materials;

    TArray<VLight> Lights;
    VLight* ShadowMakingLight;

    VSurface Environment2D;

    VCamera* Camera;
    VMesh* TerrainMesh;

    f32 YShadowPosition = -25.0f;

public:
    template<typename GameStateT = VGameState>
    void StartUp();
    void ShutDown(EWorldShutDownReason Reason = EWorldShutDownReason::Final);

    void Update(f32 DeltaTime);
    void FixedUpdate(f32 FixedDeltaTime);

    template<typename T = VEntity>
    T* SpawnEntity();
    void DestroyEntity(VEntity* Entity);

    VLight* SpawnLight(ELightType Type);
    VMaterial* AddMaterial();

    void SetEnvironment2D(const char* Path);

    template<typename GameStateT = VGameState>
    void ChangeState();

    VLN_FINLINE void SetYShadowPosition(f32 NewY)
    {
        YShadowPosition = NewY;
    }

    VLN_FINLINE VCamera* GetCamera() const
    {
        return Camera;
    }

    VLN_FINLINE void GenerateTerrain(const char* HeightMap, const char* Texture, f32 Size = 15000.0f, f32 Height = 1000.0f, EShadeMode ShadeMode = EShadeMode::Gouraud)
    {
        TerrainMesh->GenerateTerrain(HeightMap, Texture, Size, Height, ShadeMode);
    }

    VLN_FINLINE void SetShadowMakingLight(VLight* Light)
    {
        ShadowMakingLight = Light;
    }

private:
    void StartUp(VGameState* InGameState);
    void FixedUpdateEnvironment(f32 FixedDeltaTime);

    friend class VRenderer;
};

inline VWorld World;

template<typename GameStateT>
VLN_FINLINE void VWorld::StartUp()
{
    StartUp(new GameStateT());
}

template<typename T>
inline T* VWorld::SpawnEntity()
{
    VEntity* Entity = Entities.EmplaceBack(new T());
    Entity->Init();
    return (T*)Entity;
}

template<typename GameStateT>
VLN_FINLINE void VWorld::ChangeState()
{
    NextGameState = new GameStateT();
}

}