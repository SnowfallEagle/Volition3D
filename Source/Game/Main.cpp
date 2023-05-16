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
        Entity = World.SpawnEntity<VEntity>();
        LightEntity = World.SpawnEntity<VEntity>();

        // Entity->Mesh->LoadMD2("Assets/Models/tekkblade/tris.md2", nullptr, 0, {0.0f, 0.0f, 0.0f}, {10.0f, 10.0f, 10.0f}, EShadeMode::Gouraud);
        Entity->Mesh->LoadMD2("Assets/Models/monsters/brain/tris.md2", nullptr, 0, {0.0f, 0.0f, 0.0f}, {10.0f, 10.0f, 10.0f}, EShadeMode::Gouraud, { 1.5f, 2.0f, 1.5f });
        // Entity->Mesh->LoadMD2("Assets/Models/boss3/tris.md2", "Assets/Models/boss3/rider.pcx", 0, {0.0f, 0.0f, 0.0f}, {10.0f, 10.0f, 10.0f}, EShadeMode::Gouraud, { 1.5f, 2.0f, 1.5f });
        // Entity->Mesh->LoadMD2("Assets/Models/marine/tris.md2", "Assets/Models/marine/Centurion.pcx", 0, {0.0f, 0.0f, 0.0f}, {10.0f, 10.0f, 10.0f}, EShadeMode::Gouraud, { 1.5f, 2.0f, 1.5f });

        Camera = World.GetCamera();
        Camera->Init(ECameraAttr::Euler, { 0.0f, 1000.0f, 1500.0f }, { 25.0f, 180.0f, 0 }, VVector4(), 75, 100, 1000000);

        World.SetCubemap("Assets/Cubemaps/Cubemap.png");
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
            Camera->Pos.X += Math.FastSin(Camera->Dir.Y) * CamPosSpeed;
            Camera->Pos.Z += Math.FastCos(Camera->Dir.Y) * CamPosSpeed;
        }
        if (Input.IsKeyDown(EKeycode::S))
        {
            Camera->Pos.X -= Math.FastSin(Camera->Dir.Y) * CamPosSpeed;
            Camera->Pos.Z -= Math.FastCos(Camera->Dir.Y) * CamPosSpeed;
        }

        f32 CamDirSpeed = 0.1f * DeltaTime;
        if (Input.IsKeyDown(EKeycode::Left))  Camera->Dir.Y -= CamDirSpeed;
        if (Input.IsKeyDown(EKeycode::Right)) Camera->Dir.Y += CamDirSpeed;
        if (Input.IsKeyDown(EKeycode::Up))    Camera->Dir.X -= CamDirSpeed;
        if (Input.IsKeyDown(EKeycode::Down))  Camera->Dir.X += CamDirSpeed;

        if (Input.IsKeyDown(EKeycode::Space)) Camera->Pos.Y += CamPosSpeed * ShiftModifier;
        if (Input.IsKeyDown(EKeycode::C))     Camera->Pos.Y -= CamPosSpeed * ShiftModifier;

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

        if (Input.IsKeyDown(EKeycode::F12)) Config.RenderSpec.PostProcessColorCorrection = { 1.0f, 1.0f, 1.0f };
        if (Input.IsKeyDown(EKeycode::F1)) Config.RenderSpec.PostProcessColorCorrection = { 1.25f, 1.1f, 1.0f };
        if (Input.IsKeyDown(EKeycode::F2)) Config.RenderSpec.PostProcessColorCorrection = { 1.25f, 1.25f, 1.0f };
        if (Input.IsKeyDown(EKeycode::F3)) Config.RenderSpec.PostProcessColorCorrection = { 1.0f, 1.25f, 1.0f };
        if (Input.IsKeyDown(EKeycode::F4)) Config.RenderSpec.PostProcessColorCorrection = { 1.25f, 1.0f, 1.25f };
        if (Input.IsKeyDown(EKeycode::F5)) Config.RenderSpec.PostProcessColorCorrection = { 1.0f, 1.0f, 1.25f };

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

        if (Input.IsKeyDown(EKeycode::Backspace))
        {
            Config.RenderSpec.bRenderSolid ^= true;
        }

        static f32 Accum = 0.0f; 
        Accum += DeltaTime / 1000.0f;

        Renderer.DrawDebugText("FPS: %.2f", 1000.0f / DeltaTime);
        Renderer.DrawDebugText("AnimationId: %d", CurrentAnimation);
        Renderer.DrawDebugText("AnimationInterpMode: %d", (i32)InterpMode);
    }
};

class GModelsScene : public VGameState
{
public:
    virtual void StartUp() override
    {
    }

    virtual void Update(f32 DeltaTime) override
    {
    }
};

class GTerrainScene : public VGameState
{
public:
    virtual void StartUp() override
    {
        World.GetCamera()->Init(ECameraAttr::Euler, { 0.0f, 1000.0f, 1500.0f }, { 25.0f, 180.0f, 0.0f }, VVector4(), 75.0f, 100.0f, 1000000.0f);

        World.SetCubemap("Assets/Cubemaps/Cubemap.png");
        World.GetTerrain()->GenerateTerrain("Assets/Terrains/Large/Heightmap.bmp", "Assets/Terrains/RockyLand/Texture.bmp", 1000000.0f, 250000.0f, EShadeMode::Gouraud);

        World.SpawnLight(ELightType::Ambient);
        const auto OccluderLight = World.SpawnLight(ELightType::Infinite);
        World.SpawnLight(ELightType::Point);

        World.SetOccluderLight(OccluderLight);
    }

    virtual void Update(f32 DeltaTime) override
    {
        VCamera* Camera = World.GetCamera();

        f32 ShiftModifier = (Input.IsKeyDown(EKeycode::LShift) ? 4.0f : 1.0f);

        f32 CamPosSpeed = 0.5f * DeltaTime * ShiftModifier;
        if (Input.IsKeyDown(EKeycode::W))
        {
            Camera->Pos.X += Math.FastSin(Camera->Dir.Y) * CamPosSpeed;
            Camera->Pos.Z += Math.FastCos(Camera->Dir.Y) * CamPosSpeed;
        }
        if (Input.IsKeyDown(EKeycode::S))
        {
            Camera->Pos.X -= Math.FastSin(Camera->Dir.Y) * CamPosSpeed;
            Camera->Pos.Z -= Math.FastCos(Camera->Dir.Y) * CamPosSpeed;
        }

        f32 CamDirSpeed = 0.1f * DeltaTime;
        if (Input.IsKeyDown(EKeycode::Left))  Camera->Dir.Y -= CamDirSpeed;
        if (Input.IsKeyDown(EKeycode::Right)) Camera->Dir.Y += CamDirSpeed;
        if (Input.IsKeyDown(EKeycode::Up))    Camera->Dir.X -= CamDirSpeed;
        if (Input.IsKeyDown(EKeycode::Down))  Camera->Dir.X += CamDirSpeed;

        if (Input.IsKeyDown(EKeycode::M))
        {
            World.ChangeState<GTestGameState>();
        }

        Renderer.DrawDebugText("FPS: %.2f", 1000.0f / DeltaTime);
    }
};

}

int main(int Argc, char** Argv)
{
    return Engine.Run<Game::GTerrainScene>(Argc, Argv);
}