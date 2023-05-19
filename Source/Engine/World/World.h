#pragma once

#include "Common/Types/Array.h"
#include "Engine/Graphics/Scene/Light.h"
#include "Engine/Graphics/Scene/Material.h"
#include "Engine/World/Terrain.h"
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

private:
    VGameState* GameState;
    VGameState* NextGameState;

    TArray<VEntity*> Entities;
    TArray<VMaterial> Materials;

    TArray<VLight> Lights;
    VLight* ShadowMakingLight;

    VLight* LensFlareLight;
    VSurface LensFlare;

    VSurface Environment2D;
    f32 Environment2DMovementEffectAngle = 0.0f;

    VCamera* Camera;
    VTerrain* Terrain;

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

    void SetEnvironment2D(const char* Path);
    void SetEnvironment3D(const char* Front, const char* Back, const char* Top, const char* Bottom, const char* Left, const char* Right);

    VMaterial* AddMaterial();

    template<typename GameStateT = VGameState>
    void ChangeState();

    VLN_FINLINE VCamera* GetCamera() const
    {
        return Camera;
    }

    VLN_FINLINE VTerrain* GetTerrain() const
    {
        return Terrain;
    }

    VLN_FINLINE void SetShadowMakingLight(VLight* Light)
    {
        ShadowMakingLight = Light;
    }

    VLN_FINLINE void SetLensFlareLight(VLight* Light)
    {
        LensFlareLight = Light;
    }

    VLN_FINLINE void SetLensFlare(const char* Path)
    {
        LensFlare.Load(Path);
        LensFlare.SetAlphaMode(true);
    }

private:
    void StartUp(VGameState* InGameState);

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