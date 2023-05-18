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

        Renderer.RemoveTerrain();
    }

    Environment2D.Destroy();

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

    // @TODO: Camera->FixedUpdate()
    {
        Camera->Direction.Y = Math.Mod(Camera->Direction.Y, 360.0f);
        if (Camera->Direction.Y < 0.0f)
        {
            Camera->Direction.Y += 360.0f;
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

// @TODO: Clean up
#if 0
void VWorld::SetEnvironment3D(const char* Front, const char* Back, const char* Top, const char* Bottom, const char* Left, const char* Right)
{
    // @TODO: Make clean version

    VMesh* Mesh = World.SpawnEntity()->Mesh;
    Mesh->Attr &= ~EMeshAttr::CastShadow;

    // @TODO: Emissive
    static constexpr f32 Distance = 10000000.0f;

    Mesh->Allocate(4, 2, 1, 4);
    Mesh->HeadLocalVtxList[0].Position = { -Distance,   Distance,   Distance };
    Mesh->HeadLocalVtxList[1].Position = {  Distance,  -Distance,   Distance };
    Mesh->HeadLocalVtxList[2].Position = { -Distance,  -Distance,   Distance };
    Mesh->HeadLocalVtxList[3].Position = {  Distance,   Distance,   Distance };

    Mesh->TextureCoordsList[0] = { 0.0f, 0.0f };
    Mesh->TextureCoordsList[1] = { 1.0f, 1.0f };
    Mesh->TextureCoordsList[2] = { 0.0f, 1.0f };
    Mesh->TextureCoordsList[3] = { 1.0f, 1.0f };

    VMaterial* Material = World.AddMaterial();
    Material->Texture.Load(Front, { 1.0f, 1.0f, 1.0f }, 1);
    Material->Attr |= EMaterialAttr::ShadeModeTexture; // @FIXME: Two sided?

    Mesh->PolyList[0].State |= EPolyState::Active;
    Mesh->PolyList[1].State |= EPolyState::Active;

    Mesh->PolyList[0].Material = Material;
    Mesh->PolyList[1].Material = Material;

    Mesh->PolyList[0].VtxIndices[0] = 0;
    Mesh->PolyList[0].VtxIndices[1] = 1;
    Mesh->PolyList[0].VtxIndices[2] = 2;

    Mesh->PolyList[1].VtxIndices[0] = 0;
    Mesh->PolyList[1].VtxIndices[1] = 3;
    Mesh->PolyList[1].VtxIndices[2] = 1;

    Mesh->PolyList[0].TextureCoordsIndices[0] = 0;
    Mesh->PolyList[0].TextureCoordsIndices[1] = 1;
    Mesh->PolyList[0].TextureCoordsIndices[2] = 2;

    Mesh->PolyList[1].TextureCoordsIndices[0] = 0;
    Mesh->PolyList[1].TextureCoordsIndices[1] = 3;
    Mesh->PolyList[1].TextureCoordsIndices[2] = 2;
}
#endif

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
    Camera->Init(ECameraAttr::Euler, { 0.0f, 1000.0f, 1500.0f }, { 0.0f, 0.0f, 0.0f }, VVector4(), 90.0f, 250.0f, 1000000.0f);

    Terrain = new VTerrain();

    GameState = InGameState;
    GameState->StartUp();
    NextGameState = nullptr;
}

}