#include "Engine/Core/Engine.h"
#include "Common/Math/Matrix.h"

using namespace Volition;

namespace Game
{

class GTestGameState : public VGameState
{
    VEntity* Entity = nullptr;
    VEntity* LightEntity = nullptr;
    VCamera* Camera;

    u8 CurrentAnimation = 0;
    EAnimationInterpMode InterpMode = EAnimationInterpMode::Default;

    VVector4 OccluderLightPosition;

public:
    virtual void StartUp() override
    {
        const auto COBEntity = World.SpawnEntity()->Mesh->LoadCOB("Assets/Models/jetski05.cob", { 1000.0f, 1000.0f, 0.0f }, { 100.0f, 100.0f, 100.0f }, ECOBFlags::Default | ECOBFlags::InvertV /* | ECOBFlags::OverrideShadeMode */, EShadeMode::Gouraud);

        Entity = World.SpawnEntity<VEntity>();
        LightEntity = World.SpawnEntity<VEntity>();

        // Entity->Mesh->LoadMD2("Assets/Models/tekkblade/tris.md2", nullptr, 0, {0.0f, 0.0f, 0.0f}, {10.0f, 10.0f, 10.0f}, EShadeMode::Gouraud);
        Entity->Mesh->LoadMD2("Assets/Models/monsters/brain/tris.md2", nullptr, 0, {0.0f, 0.0f, 0.0f}, {10.0f, 10.0f, 10.0f}, EShadeMode::Gouraud, { 1.5f, 2.0f, 1.5f });
        // Entity->Mesh->LoadMD2("Assets/Models/boss3/tris.md2", "Assets/Models/boss3/rider.pcx", 0, {0.0f, 0.0f, 0.0f}, {10.0f, 10.0f, 10.0f}, EShadeMode::Gouraud, { 1.5f, 2.0f, 1.5f });
        // Entity->Mesh->LoadMD2("Assets/Models/marine/tris.md2", "Assets/Models/marine/Centurion.pcx", 0, {0.0f, 0.0f, 0.0f}, {10.0f, 10.0f, 10.0f}, EShadeMode::Gouraud, { 1.5f, 2.0f, 1.5f });

        Camera = World.GetCamera();
        Camera->Init(ECameraAttr::Euler, { 0.0f, 1000.0f, 1500.0f }, { 25.0f, 180.0f, 0 }, VVector4(), 75, 100, 1000000);

        World.SetEnvironment2D("Assets/Environment2D/Texture.png");
        // World.GetTerrain()->GenerateTerrain("Assets/Terrains/Large/Heightmap.bmp", "Assets/Terrains/RockyLand/Texture.bmp", 1000000.0f, 250000.0f, EShadeMode::Gouraud);
        World.GetTerrain()->GenerateTerrain("Assets/Terrains/Medium/Heightmap.bmp", "Assets/Terrains/Common/Texture.bmp", 10000.0f, 2500.0f, EShadeMode::Gouraud);

        World.SpawnLight(ELightType::Ambient);
        const auto OccluderLight = World.SpawnLight(ELightType::Infinite);
        World.SpawnLight(ELightType::Point);

        World.SetOccluderLight(OccluderLight);
    }

    virtual void Update(f32 DeltaTime) override
    {
        if (Input.IsKeyDown(EKeycode::Escape))
        {
            Engine.Stop();
        }

        f32 ShiftModifier = (Input.IsKeyDown(EKeycode::LShift) ? 4.0f : 1.0f);

        f32 CamPosSpeed = 0.5f * DeltaTime * ShiftModifier;
        if (Input.IsKeyDown(EKeycode::W))
        {
            Camera->Position.X += Math.FastSin(Camera->Direction.Y) * CamPosSpeed;
            Camera->Position.Z += Math.FastCos(Camera->Direction.Y) * CamPosSpeed;
        }
        if (Input.IsKeyDown(EKeycode::S))
        {
            Camera->Position.X -= Math.FastSin(Camera->Direction.Y) * CamPosSpeed;
            Camera->Position.Z -= Math.FastCos(Camera->Direction.Y) * CamPosSpeed;
        }

        f32 CamDirSpeed = 0.1f * DeltaTime;
        if (Input.IsKeyDown(EKeycode::Left))  Camera->Direction.Y -= CamDirSpeed;
        if (Input.IsKeyDown(EKeycode::Right)) Camera->Direction.Y += CamDirSpeed;
        if (Input.IsKeyDown(EKeycode::Up))    Camera->Direction.X -= CamDirSpeed;
        if (Input.IsKeyDown(EKeycode::Down))  Camera->Direction.X += CamDirSpeed;

        if (Input.IsKeyDown(EKeycode::Space)) Camera->Position.Y += CamPosSpeed * ShiftModifier;
        if (Input.IsKeyDown(EKeycode::C))     Camera->Position.Y -= CamPosSpeed * ShiftModifier;

        VMatrix44 Rot = VMatrix44::Identity;
        f32 Speed = 0.05f * DeltaTime;
        if (Input.IsKeyDown(EKeycode::Q)) Rot.BuildRotationXYZ(0, Speed, 0);
        if (Input.IsKeyDown(EKeycode::E)) Rot.BuildRotationXYZ(0, -Speed, 0);
        if (Input.IsKeyDown(EKeycode::F)) Rot.BuildRotationXYZ(0, 0, Speed);
        if (Input.IsKeyDown(EKeycode::G)) Rot.BuildRotationXYZ(0, 0, -Speed);
        if (Input.IsKeyDown(EKeycode::Z)) Rot.BuildRotationXYZ(Speed, 0, 0);
        if (Input.IsKeyDown(EKeycode::X)) Rot.BuildRotationXYZ(-Speed, 0, 0);

        if (Input.IsKeyDown(EKeycode::P)) Entity->Mesh->PlayAnimation((EMD2AnimationId)CurrentAnimation, false, InterpMode);
        if (Input.IsKeyDown(EKeycode::L)) Entity->Mesh->PlayAnimation((EMD2AnimationId)CurrentAnimation, true, InterpMode);

        if (Input.IsKeyDown(EKeycode::N0)) InterpMode = EAnimationInterpMode::Default;
        if (Input.IsKeyDown(EKeycode::N1)) InterpMode = EAnimationInterpMode::Linear;
        if (Input.IsKeyDown(EKeycode::N2)) InterpMode = EAnimationInterpMode::Fixed;

        if (Input.IsKeyDown(EKeycode::Home)) Entity->Mesh->Position.Z += DeltaTime * ShiftModifier;
        if (Input.IsKeyDown(EKeycode::End)) Entity->Mesh->Position.Z -= DeltaTime * ShiftModifier;
        if (Input.IsKeyDown(EKeycode::Delete)) Entity->Mesh->Position.X -= DeltaTime * ShiftModifier;
        if (Input.IsKeyDown(EKeycode::PageDown)) Entity->Mesh->Position.X += DeltaTime * ShiftModifier;

        static float AnimButtonCounter = 0.0f;
        if (AnimButtonCounter <= 0.0f && Input.IsKeyDown(EKeycode::Comma)) { --CurrentAnimation; AnimButtonCounter = 100.0f; };
        if (AnimButtonCounter <= 0.0f && Input.IsKeyDown(EKeycode::Period)) { ++CurrentAnimation; AnimButtonCounter = 100.0f; };

        if (AnimButtonCounter >= 0.0f)
        {
            AnimButtonCounter -= DeltaTime;
        }
        CurrentAnimation %= (i32)EMD2AnimationId::MaxAnimations;

        static b32 bStarted = false;
        if (Input.IsKeyDown(EKeycode::B)) bStarted ^= true;

        if (bStarted && Entity->Mesh->bAnimationPlayed)
        {
            Entity->Mesh->PlayAnimation((EMD2AnimationId)CurrentAnimation++, false, InterpMode);
            CurrentAnimation %= (i32)EMD2AnimationId::MaxAnimations;
        }

        if (Entity)
        {
            Entity->Mesh->Transform(Rot, ETransformType::LocalOnly, true);
        }

        if (Input.IsKeyDown(EKeycode::Backspace)) Config.RenderSpec.bRenderSolid ^= true;

        if (Input.IsKeyDown(EKeycode::F12)) Config.RenderSpec.PostProcessColorCorrection = { 1.0f, 1.0f, 1.0f };
        if (Input.IsKeyDown(EKeycode::F1)) Config.RenderSpec.PostProcessColorCorrection = { 1.25f, 1.1f, 1.0f };
        if (Input.IsKeyDown(EKeycode::F2)) Config.RenderSpec.PostProcessColorCorrection = { 1.25f, 1.25f, 1.0f };
        if (Input.IsKeyDown(EKeycode::F3)) Config.RenderSpec.PostProcessColorCorrection = { 1.0f, 1.25f, 1.0f };
        if (Input.IsKeyDown(EKeycode::F4)) Config.RenderSpec.PostProcessColorCorrection = { 1.25f, 1.0f, 1.25f };
        if (Input.IsKeyDown(EKeycode::F5)) Config.RenderSpec.PostProcessColorCorrection = { 1.0f, 1.0f, 1.25f };
        static f32 Accum = 0.0f; 
        Accum += DeltaTime / 1000.0f;

        Renderer.DrawDebugText("FPS: %.2f", 1000.0f / DeltaTime);
        Renderer.DrawDebugText("AnimationId: %d", CurrentAnimation);
        Renderer.DrawDebugText("AnimationInterpMode: %d", (i32)InterpMode);
    }
};

class GGameState : public VGameState
{
protected:
    virtual void StartUp() override
    {
        World.SetEnvironment2D("Assets/Environment2D/Texture.png");

        World.SpawnLight(ELightType::Ambient);
        const auto OccluderLight = World.SpawnLight(ELightType::Infinite);
        World.SpawnLight(ELightType::Point);

        World.SetOccluderLight(OccluderLight);

        Config.RenderSpec.PostProcessColorCorrection = { 1.25f, 1.1f, 1.0f };
    }

    virtual void Update(f32 DeltaTime) override
    {
        ProcessInput(DeltaTime);

        Renderer.DrawDebugText("FPS: %.2f", 1000.0f / DeltaTime);
    }

    virtual void ProcessInput(f32 DeltaTime);
};

class GModelsScene : public GGameState
{
public:
    using Super = GGameState;

protected:
    virtual void StartUp() override
    {
        Super::StartUp();

        // @TODO: Clean up
        #if 0
        World.SetEnvironment3D("Assets/Environment3D/Sky/null_plainsky256_ft.pcx", nullptr, nullptr, nullptr, nullptr, nullptr);
        #endif

        World.GetCamera()->ZFarClip = 1000000000.0f;
    }

    virtual void Update(f32 DeltaTime) override
    {
        Super::Update(DeltaTime);
    }
};

class GLargeTerrainScene : public GGameState
{
public:
    using Super = GGameState;

protected:
    virtual void StartUp() override
    {
        Super::StartUp();

        VCamera* Camera = World.GetCamera();
        Camera->Direction = { 15.0f, 180.0f, 0.0f };
        Camera->ZFarClip = 3000000.0f;

        World.GetTerrain()->GenerateTerrain("Assets/Terrains/Large/Heightmap.bmp", "Assets/Terrains/RockyLand/Texture.bmp", 1000000.0f, 250000.0f, EShadeMode::Gouraud);
    }
};

class GGrandTerrainScene : public GGameState
{
public:
    using Super = GGameState;

protected:
    virtual void StartUp() override
    {
        Super::StartUp();

        VCamera* Camera = World.GetCamera();
        Camera->Position = { 50000.0f, 0.0f, -500000.0f };
        Camera->Direction = { 45.0f, -15.0f, 0.0f };
        Camera->ZFarClip = 5000000.0f;
        World.GetTerrain()->GenerateTerrain("Assets/Terrains/Grand/Heightmap.png", "Assets/Terrains/Grand/Texture.png", 2000000.0f, 2000000.0f, EShadeMode::Gouraud);
    }
};

void GGameState::ProcessInput(f32 DeltaTime)
{
    VCamera* Camera = World.GetCamera();

    f32 ShiftModifier = (Input.IsKeyDown(EKeycode::LShift) ? 150.0f : 1.0f);

    f32 CamPosSpeed = 0.5f * DeltaTime * ShiftModifier;
    if (Input.IsKeyDown(EKeycode::W))
    {
        Camera->Position.X += Math.FastSin(Camera->Direction.Y) * CamPosSpeed;
        Camera->Position.Z += Math.FastCos(Camera->Direction.Y) * CamPosSpeed;
    }
    if (Input.IsKeyDown(EKeycode::S))
    {
        Camera->Position.X -= Math.FastSin(Camera->Direction.Y) * CamPosSpeed;
        Camera->Position.Z -= Math.FastCos(Camera->Direction.Y) * CamPosSpeed;
    }

    if (Input.IsKeyDown(EKeycode::Space)) Camera->Position.Y += CamPosSpeed * 5.0f;
    if (Input.IsKeyDown(EKeycode::C))     Camera->Position.Y -= CamPosSpeed * 5.0f;

    f32 CamDirSpeed = 0.1f * DeltaTime;
    if (Input.IsKeyDown(EKeycode::Left))  Camera->Direction.Y -= CamDirSpeed;
    if (Input.IsKeyDown(EKeycode::Right)) Camera->Direction.Y += CamDirSpeed;
    if (Input.IsKeyDown(EKeycode::Up))    Camera->Direction.X -= CamDirSpeed;
    if (Input.IsKeyDown(EKeycode::Down))  Camera->Direction.X += CamDirSpeed;

    if (Input.IsKeyDown(EKeycode::Backspace)) Config.RenderSpec.bRenderSolid ^= true;

    if (Input.IsKeyDown(EKeycode::F1)) World.ChangeState<GLargeTerrainScene>();
    if (Input.IsKeyDown(EKeycode::F2)) World.ChangeState<GGrandTerrainScene>();
    if (Input.IsKeyDown(EKeycode::F3)) World.ChangeState<GModelsScene>();
    if (Input.IsKeyDown(EKeycode::F4)) World.ChangeState<GTestGameState>();

    if (Input.IsKeyDown(EKeycode::F12)) Config.RenderSpec.PostProcessColorCorrection = { 1.0f, 1.0f, 1.0f };
    if (Input.IsKeyDown(EKeycode::F5)) Config.RenderSpec.PostProcessColorCorrection = { 1.25f, 1.1f, 1.0f };
    if (Input.IsKeyDown(EKeycode::F6)) Config.RenderSpec.PostProcessColorCorrection = { 1.25f, 1.25f, 1.0f };
    if (Input.IsKeyDown(EKeycode::F7)) Config.RenderSpec.PostProcessColorCorrection = { 1.0f, 1.25f, 1.0f };
    if (Input.IsKeyDown(EKeycode::F8)) Config.RenderSpec.PostProcessColorCorrection = { 1.25f, 1.0f, 1.25f };
    if (Input.IsKeyDown(EKeycode::F9)) Config.RenderSpec.PostProcessColorCorrection = { 1.0f, 1.0f, 1.25f };

    if (Input.IsKeyDown(EKeycode::Escape)) Engine.Stop();
}

}

int main(int Argc, char** Argv)
{
    return Engine.Run<Game::GModelsScene>(Argc, Argv);
}