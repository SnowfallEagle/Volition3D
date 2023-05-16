#include "Engine/Graphics/Rendering/Renderer.h"
#include "Engine/World/World.h"

namespace Volition
{

void VWorld::ShutDown(EWorldShutDownReason Reason)
{
    if (GameState)
    {
        GameState->ShutDown();
        delete GameState;
        GameState = nullptr;
    }

    if (Reason == EWorldShutDownReason::Final)
    {
        VLN_SAFE_DELETE(NextGameState);
        VLN_SAFE_DELETE(Camera);
    }

    if (Terrain)
    {
        Terrain->Destroy();
        delete Terrain;
        Terrain = nullptr;
    }

    Cubemap.Destroy();

    for (const auto Entity : Entities)
    {
        if (Entity)
        {
            Entity->Destroy();
            delete Entity;
        }
    }
    Entities.Clear();

    for (auto& Material : Materials)
    {
        Material.Destroy();
    }
    Materials.Clear();
}

void VWorld::Update(f32 DeltaTime)
{
    // Check new state
    if (NextGameState)
    {
        World.ShutDown(EWorldShutDownReason::Reset);
        World.StartUp(NextGameState);
    }

    // Update world stuff
    GameState->Update(DeltaTime);

    // @TODO: Camera->Update()
    {
        Camera->Dir.Y = Math.Mod(Camera->Dir.Y, 360.0f);
        if (Camera->Dir.Y < 0.0f)
        {
            Camera->Dir.Y += 360.0f;
        }
    }

    for (const auto Entity : Entities)
    {
        if (Entity)
        {
            Entity->Update(DeltaTime);
        }
    }
}

void VWorld::FixedUpdate(f32 FixedDeltaTime)
{
    static constexpr f32 CubemapMovementEffectSpeed = 0.00175f;
    CubemapMovementEffectAngle = Math.Mod(CubemapMovementEffectAngle + (CubemapMovementEffectSpeed * FixedDeltaTime), 360.0f);
    if (CubemapMovementEffectAngle < 0.0f)
    {
        CubemapMovementEffectAngle += 360.0f;
    }
}

void VWorld::DestroyEntity(VEntity* Entity)
{
    if (Entity)
    {
        std::remove(Entities.begin(), Entities.end(), Entity);
        Entity->Destroy();
        delete Entity;
    }
}

void VWorld::SetCubemap(const char* Path)
{
    Cubemap.Load(Path);
}

VLight* VWorld::SpawnLight(ELightType Type)
{
    VLight& Light = Lights.EmplaceBack(VLight());
    Light.Init(Type);
    return &Light;
}

VMaterial* VWorld::AddMaterial()
{
    VMaterial& Material = Materials.EmplaceBack(VMaterial());
    Material.Init();
    return &Material;
}

void VWorld::StartUp(VGameState* InGameState)
{
    Entities.Resize(MinEntitiesCapacity);
    Materials.Resize(MinMaterialsCapacity);

    Lights.Resize(MinLightsCapacity);
    OccluderLight = nullptr;

    Camera = new VCamera();
    Camera->Init(ECameraAttr::Euler, { 0.0f, 1000.0f, 1500.0f }, { 25.0f, 180.0f, 0.0f }, VVector4(), 90.0f, 100.0f, 1000000.0f);

    Terrain = new VTerrain();

    GameState = InGameState;
    GameState->StartUp();
    NextGameState = nullptr;
}

}