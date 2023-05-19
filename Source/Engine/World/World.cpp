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

    if (TerrainMesh)
    {
        TerrainMesh->Destroy();
        delete TerrainMesh;
        TerrainMesh = nullptr;

        Renderer.RemoveTerrain();
    }

    Environment2D.Destroy();
    LensFlare.Destroy();

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

    for (const auto Entity : Entities)
    {
        if (Entity)
        {
            Entity->Update(DeltaTime);
        }
    }

    Camera->Update(DeltaTime);
}

void VWorld::FixedUpdate(f32 FixedDeltaTime)
{
    GameState->FixedUpdate(FixedDeltaTime);

    // @TODO: Environment2D->FixedUpdate()
    static constexpr f32 Environment2DMovementEffectSpeed = 0.00175f;
    Environment2DMovementEffectAngle = Math.Mod(Environment2DMovementEffectAngle + (Environment2DMovementEffectSpeed * FixedDeltaTime), 360.0f);
    if (Environment2DMovementEffectAngle < 0.0f)
    {
        Environment2DMovementEffectAngle += 360.0f;
    }
}

void VWorld::DestroyEntity(VEntity* Entity)
{
    if (Entity)
    {
        Entities.Remove(Entity);
        Entity->Destroy();
        delete Entity;
    }
}

void VWorld::SetEnvironment2D(const char* Path)
{
    Environment2D.Load(Path);
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
    ShadowMakingLight = nullptr;
    LensFlareLight = nullptr;

    Camera = new VCamera();
    Camera->Init(ECameraAttr::Euler, { 0.0f, 1000.0f, 1500.0f }, { 0.0f, 0.0f, 0.0f }, VVector4(), 90.0f, 250.0f, 1000000.0f);

    TerrainMesh = new VMesh();

    GameState = InGameState;
    GameState->StartUp();
    NextGameState = nullptr;
}

}