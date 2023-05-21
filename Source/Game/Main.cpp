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
        World.GenerateTerrain("Assets/Terrains/Medium/Heightmap.bmp", "Assets/Terrains/Common/Texture.bmp", 10000.0f, 2500.0f, EShadeMode::Gouraud);

        World.SpawnLight(ELightType::Ambient);
        const auto ShadowMakingLight = World.SpawnLight(ELightType::Infinite);
        World.SpawnLight(ELightType::Point);

        World.SetShadowMakingLight(ShadowMakingLight);
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
    VVector2 MouseMoveAccum = { 0.0f, 0.0f };

    f32 ShiftCamPosSpeedModifier = 5.0f;
    f32 CamPosSpeedModifier      = 2.5f;
    f32 CamDirSpeedModifier      = 0.5f;

    f32 MouseSensivity = 0.05f;
    f32 MaxMouseDelta  = 30.0f;

    VVector4 StartSunLightPosition;

    VLight* AmbientLight;
    VLight* PointLight;
    VLight* SunLight;

protected:
    virtual void StartUp() override
    {
        World.SetEnvironment2D("Assets/Environment2D/Texture.png");
        World.SetLensFlare("Assets/Textures/SunFlare.png");

        AmbientLight = World.SpawnLight(ELightType::Ambient);
        PointLight = World.SpawnLight(ELightType::Point);

        SunLight = World.SpawnLight(ELightType::Infinite);
        StartSunLightPosition = SunLight->Position;

        World.SetShadowMakingLight(SunLight);
        World.SetLensFlareLight(SunLight);
        World.Environment2DMovementEffectAngle = 165.0f;

        Config.RenderSpec.PostProcessColorCorrection = { 1.25f, 1.1f, 1.0f };
    }

    virtual void Update(f32 DeltaTime) override
    {
        ProcessInput(DeltaTime);

        Renderer.DrawDebugText("FPS: %.2f", 1000.0f / DeltaTime);
    }


    virtual void FixedUpdate(f32 FixedDeltaTime) override
    {
        // Rotate sun light
        VMatrix44 MatRotation;
        MatRotation.BuildRotationXYZ(0.0f, -World.Environment2DMovementEffectAngle, 0.0f);

        VMatrix44::MulVecMat(StartSunLightPosition, MatRotation, SunLight->Position);
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

        VCamera* Camera = World.GetCamera();
        Camera->ZFarClip = 1000000000.0f;

        const auto COBEntity = World.SpawnEntity()->Mesh->LoadCOB("Assets/Models/jetski05.cob", { 1000.0f, 1000.0f, 0.0f }, { 100.0f, 100.0f, 100.0f }, ECOBFlags::Default | ECOBFlags::InvertV /* | ECOBFlags::OverrideShadeMode */, EShadeMode::Gouraud);

        const auto Entity = World.SpawnEntity<VEntity>();
        // Entity->Mesh->LoadMD2("Assets/Models/monsters/brain/tris.md2", nullptr, 0, {0.0f, 0.0f, 0.0f}, {10.0f, 10.0f, 10.0f}, EShadeMode::Gouraud, { 1.5f, 2.0f, 1.5f });
        // Entity->Mesh->LoadMD2("Assets/Models/blade/tris.md2", "Assets/Models/blade/blade.pcx", 0, {0.0f, 0.0f, 0.0f}, {10.0f, 10.0f, 10.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Entity->Mesh->LoadMD2("Assets/Models/bobafett/tris.md2", "Assets/Models/bobafett/rotj_fett.pcx", 0, {0.0f, 0.0f, 0.0f}, {10.0f, 10.0f, 10.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        // Entity->Mesh->LoadMD2("Assets/Models/0069/tris.md2", "Assets/Models/0069/actionbond.pcx", 0, {0.0f, 0.0f, 0.0f}, {10.0f, 10.0f, 10.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});

        Camera = World.GetCamera();
        Camera->Init(ECameraAttr::Euler, { 0.0f, 1000.0f, 1500.0f }, { 25.0f, 180.0f, 0.0f }, VVector4(), 90.0f, 100.0f, 1000000.0f);

        World.GenerateTerrain("Assets/Terrains/Medium/Heightmap.bmp", "Assets/Terrains/Common/Texture.bmp", 10000.0f, 2500.0f, EShadeMode::Gouraud);

        const auto Spotlight = World.SpawnLight(ELightType::ComplexSpotlight);
        Spotlight->Position = { 0.0f, 1500.0f, -100.0f };
        Spotlight->Color = MAP_XRGB32(0xFF, 0x00, 0x11);
        Spotlight->KQuad = 0.0000001f;
        Spotlight->KLinear = 0.0f;
        Spotlight->FalloffPower = 5.0f;

        World.SetShadowMakingLight(Spotlight);
    }

    virtual void Update(f32 DeltaTime) override
    {
        Super::Update(DeltaTime);
    }
};

class GAgentWithBladeScene : public GGameState
{
public:
    using Super = GGameState;

protected:
    virtual void StartUp() override
    {
        Super::StartUp();

        Config.RenderSpec.PostProcessColorCorrection = { 0.6f, 0.6f, 1.0f };
        Config.RenderSpec.bRenderUI = false;

        PointLight->bActive = false;
        SunLight->Color.R -= 0x11;
        SunLight->Color.G -= 0x11;

        const auto Spotlight = World.SpawnLight(ELightType::ComplexSpotlight);
        Spotlight->Position = { 0.0f, -10000.0f, 250.0f };
        Spotlight->Direction = VVector4{ 0.0f, -0.75f, -0.5f }.GetNormalized();
        Spotlight->Color = MAP_XRGB32(0x88, 0x88, 0xAA);
        Spotlight->KQuad = 0.0000001f;
        Spotlight->KLinear = 0.0f;
        Spotlight->FalloffPower = 5.0f;

        const auto Spotlight2 = World.SpawnLight(ELightType::ComplexSpotlight);
        Spotlight2->Position = { 0.0f, -10000.0f, -250.0f };
        Spotlight2->Direction = VVector4{ 0.0f, -0.5f, 1.0f }.GetNormalized();
        Spotlight2->Color = MAP_XRGB32(0xAA, 0x22, 0x22);
        Spotlight2->KQuad = 0.00000005f;
        Spotlight2->KLinear = 0.0f;
        Spotlight2->FalloffPower = 5.0f;

        const auto CornerLight = World.SpawnLight(ELightType::ComplexSpotlight);
        CornerLight->Position = { -7500.0f, -5000.0f, -10500.0f };
        CornerLight->Direction = VVector4{ -0.25f, -1.0f, -1.0f }.GetNormalized();
        CornerLight->Color = MAP_XRGB32(0xAA, 0x33, 0x33);
        CornerLight->KQuad = 0.0000000000005f;
        CornerLight->FalloffPower = 5.0f;

        World.SetYShadowPosition(-12200.0f);
        World.GenerateTerrain("Assets/Terrains/Medium/Heightmap.bmp", "Assets/Terrains/Common/Blue.bmp", 50000.0f, 25000.0f, EShadeMode::Gouraud);

        const auto Agent = World.SpawnEntity<VEntity>();
        Agent->Mesh->LoadMD2("Assets/Models/0069/tris.md2", "Assets/Models/0069/actionbond.pcx", 0, {260.0f, -12200.0f, 1000.0f}, { 20.0f, 20.0f, 20.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Agent->Mesh->PlayAnimation(EMD2AnimationId::StandingIdle, true);
        Agent->Mesh->Rotation = { 0.0f, 180.0f, 0.0f };

        const auto AgentWeapon = World.SpawnEntity<VEntity>();
        AgentWeapon->Mesh->LoadMD2("Assets/Models/0069/weapon.md2", "Assets/Models/0069/weapon.pcx", 0, {Agent->Mesh->Position.X + 40.0f, Agent->Mesh->Position.Y + 600.0f, Agent->Mesh->Position.Z - 80.0f}, { 20.0f, 20.0f, 20.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        AgentWeapon->Mesh->PlayAnimation(EMD2AnimationId::StandingIdle, true);
        AgentWeapon->Mesh->Rotation = { 0.0f, 180.0f, 0.0f };

        const auto Trooper1 = World.SpawnEntity<VEntity>();
        Trooper1->Mesh->LoadMD2("Assets/Models/Marine/tris.md2", "Assets/Models/Marine/Centurion.pcx", 0, {0.0f, -12200.0f, 4000.0f}, { 20.0f, 20.0f, 20.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Trooper1->Mesh->PlayAnimation(EMD2AnimationId::CrouchStand, true);
        Trooper1->Mesh->Rotation = { 0.0f, 180.0f, 0.0f };

        const auto Trooper2 = World.SpawnEntity<VEntity>();
        Trooper2->Mesh->LoadMD2("Assets/Models/Marine/tris.md2", "Assets/Models/Marine/Centurion.pcx", 0, {1000.0f, -12200.0f, 4000.0f}, { 20.0f, 20.0f, 20.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Trooper2->Mesh->PlayAnimation(EMD2AnimationId::StandingIdle, true);
        Trooper2->Mesh->Rotation = { 0.0f, 210.0f, 0.0f };

        const auto Trooper3 = World.SpawnEntity<VEntity>();
        Trooper3->Mesh->LoadMD2("Assets/Models/Marine/tris.md2", "Assets/Models/Marine/Centurion.pcx", 0, {-1000.0f, -12200.0f, 4000.0f}, { 20.0f, 20.0f, 20.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Trooper3->Mesh->PlayAnimation(EMD2AnimationId::StandingIdle, true);
        Trooper3->Mesh->AnimationTimeAccum += 1.0f;
        Trooper3->Mesh->Rotation = { 0.0f, 150.0f, 0.0f };

        const auto Trooper4 = World.SpawnEntity<VEntity>();
        Trooper4->Mesh->LoadMD2("Assets/Models/Marine/tris.md2", "Assets/Models/Marine/Centurion.pcx", 0, {-8000.0f, -12200.0f, -15000.0f}, { 20.0f, 20.0f, 20.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Trooper4->Mesh->PlayAnimation(EMD2AnimationId::Wave, true);
        Trooper4->Mesh->Rotation = { 0.0f, 30.0f, 0.0f };

        const auto Trooper5 = World.SpawnEntity<VEntity>();
        Trooper5->Mesh->LoadMD2("Assets/Models/Marine/tris.md2", "Assets/Models/Marine/Centurion.pcx", 0, {-6000.0f, -12200.0f, -15000.0f}, { 20.0f, 20.0f, 20.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Trooper5->Mesh->PlayAnimation(EMD2AnimationId::Salute, true);
        Trooper5->Mesh->Rotation = { 0.0f, 00.0f, 0.0f };

        const auto Blade = World.SpawnEntity<VEntity>();
        Blade->Mesh->LoadMD2("Assets/Models/blade/tris.md2", "Assets/Models/blade/blade.pcx", 0, {0.0f, -12200.0f, 0.0f}, {20.0f, 20.0f, 20.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Blade->Mesh->PlayAnimation(EMD2AnimationId::StandingIdle, true);

        const auto Dead = World.SpawnEntity<VEntity>();
        Dead->Mesh->LoadMD2("Assets/Models/deadbods/dude/tris.md2", "Assets/Models/deadbods/dude/dead1.pcx", 0, { 5000.0f, -12200.0f, 4750.0f }, { 15.0f, 15.0f, 15.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Dead->Mesh->Rotation.Y = 90.0f;

        World.GetCamera()->Init(ECameraAttr::Euler, {0.0f, 1000.0f, 1500.0f}, {25.0f, 180.0f, 0.0f}, VVector4(), 90.0f, 75.0f, 1000000.0f);

        CamPosSpeedModifier *= 0.25f;
    }

    virtual void Update(f32 DeltaTime) override
    {
        Super::Update(DeltaTime);
    }
};

class GRaidScene : public GGameState
{
public:
    using Super = GGameState;

protected:
    virtual void StartUp() override
    {
        Super::StartUp();

        Config.RenderSpec.PostProcessColorCorrection = { 1.0f, 1.0f, 1.1f };
        Config.RenderSpec.bRenderUI = false;

        const auto Spotlight = World.SpawnLight(ELightType::ComplexSpotlight);
        Spotlight->Position = { 0.0f, -10000.0f, 250.0f };
        Spotlight->Direction = VVector4{ 0.0f, -0.75f, -0.5f }.GetNormalized();
        Spotlight->Color = MAP_XRGB32(0x88, 0x88, 0xAA);
        Spotlight->KQuad = 0.0000001f;
        Spotlight->KLinear = 0.0f;
        Spotlight->FalloffPower = 5.0f;

        const auto Spotlight2 = World.SpawnLight(ELightType::ComplexSpotlight);
        Spotlight2->Position = { 0.0f, -10000.0f, -250.0f };
        Spotlight2->Direction = VVector4{ 0.0f, -0.5f, 1.0f }.GetNormalized();
        Spotlight2->Color = MAP_XRGB32(0xAA, 0x22, 0x22);
        Spotlight2->KQuad = 0.00000005f;
        Spotlight2->KLinear = 0.0f;
        Spotlight2->FalloffPower = 5.0f;

        const auto CornerLight = World.SpawnLight(ELightType::ComplexSpotlight);
        CornerLight->Position = { -7500.0f, -5000.0f, -10500.0f };
        CornerLight->Direction = VVector4{ -0.25f, -1.0f, -1.0f }.GetNormalized();
        CornerLight->Color = MAP_XRGB32(0xAA, 0x33, 0x33);
        CornerLight->KQuad = 0.0000000000005f;
        CornerLight->FalloffPower = 5.0f;

        World.SetYShadowPosition(-6000.0f);
        World.GenerateTerrain("Assets/Terrains/RockyLand/Heightmap.bmp", "Assets/Terrains/RockyLand/Texture.bmp", 500000.0f, 50000.0f, EShadeMode::Gouraud);
        StartSunLightPosition = {1000000.0f, 1000000.0f, 1000000.0f};

        const auto Trooper = World.SpawnEntity<VEntity>();
        Trooper->Mesh->LoadMD2("Assets/Models/bobafett/tris.md2", "Assets/Models/bobafett/rotj_fett.pcx", 0, {-8000.0f, -6000.0f, -15000.0f}, { 20.0f, 20.0f, 20.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Trooper->Mesh->PlayAnimation(EMD2AnimationId::Run, true);
        Trooper->Mesh->Rotation = { 0.0f, 30.0f, 0.0f };

        World.GetCamera()->Init(ECameraAttr::Euler, {-10000.0f, -5500.0f, 1500.0f}, {-15.0f, 180.0f, 0.0f}, VVector4(), 90.0f, 75.0f, 1000000.0f);
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

        CamPosSpeedModifier = 25.0f;

        World.GenerateTerrain("Assets/Terrains/Large/Heightmap.bmp", "Assets/Terrains/RockyLand/Texture.bmp", 1000000.0f, 250000.0f, EShadeMode::Gouraud);
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

        CamPosSpeedModifier = 25.0f;
        CamDirSpeedModifier = 0.05f;

        World.GenerateTerrain("Assets/Terrains/Grand/Heightmap.png", "Assets/Terrains/Grand/Texture.png", 2000000.0f, 2000000.0f, EShadeMode::Gouraud);
    }
};

void GGameState::ProcessInput(f32 DeltaTime)
{
    VCamera* Camera = World.GetCamera();

    f32 CamPosSpeed = CamPosSpeedModifier * DeltaTime * (Input.IsKeyDown(EKeycode::LShift) ? ShiftCamPosSpeedModifier : 1.0f);
    if (Input.IsKeyDown(EKeycode::W))
    {
        Camera->Position.X += Math.FastSin(Camera->Direction.Y) * CamPosSpeed;
        Camera->Position.Y -= Math.FastSin(Camera->Direction.X) * CamPosSpeed;
        Camera->Position.Z += Math.FastCos(Camera->Direction.Y) * CamPosSpeed;
    }
    if (Input.IsKeyDown(EKeycode::S))
    {
        Camera->Position.X -= Math.FastSin(Camera->Direction.Y) * CamPosSpeed;
        Camera->Position.Y += Math.FastSin(Camera->Direction.X) * CamPosSpeed;
        Camera->Position.Z -= Math.FastCos(Camera->Direction.Y) * CamPosSpeed;
    }
    if (Input.IsKeyDown(EKeycode::A))
    {
        const f32 Angle = Camera->Direction.Y - 90.0f;
        Camera->Position.X += Math.FastSin(Angle) * CamPosSpeed;
        Camera->Position.Z += Math.FastCos(Angle) * CamPosSpeed;
    }
    if (Input.IsKeyDown(EKeycode::D))
    {
        const f32 Angle = Camera->Direction.Y + 90.0f;
        Camera->Position.X += Math.FastSin(Angle) * CamPosSpeed;
        Camera->Position.Z += Math.FastCos(Angle) * CamPosSpeed;
    }

    if (Input.IsKeyDown(EKeycode::Space)) Camera->Position.Y += CamPosSpeed;
    if (Input.IsKeyDown(EKeycode::C))     Camera->Position.Y -= CamPosSpeed;

    f32 CamDirSpeed = DeltaTime * CamDirSpeedModifier;
    if (Input.IsKeyDown(EKeycode::Left))  MouseMoveAccum.X -= CamDirSpeed;
    if (Input.IsKeyDown(EKeycode::Right)) MouseMoveAccum.X += CamDirSpeed;
    if (Input.IsKeyDown(EKeycode::Up))    MouseMoveAccum.Y -= CamDirSpeed;
    if (Input.IsKeyDown(EKeycode::Down))  MouseMoveAccum.Y += CamDirSpeed;

    VVector2i RelMove = Input.GetMouseRelativePosition();
    MouseMoveAccum.X += (f32)RelMove.X;
    MouseMoveAccum.Y += (f32)RelMove.Y;

    static constexpr f32 Divider = 0.5f;
    const VVector2i MouseMoveInt = { (i32)((f32)MouseMoveAccum.X / Divider), (i32)((f32)MouseMoveAccum.Y / Divider) };

    const f32 Multiplier = DeltaTime * MouseSensivity; 

    VVector2 MouseMoveFloat = { MouseMoveInt.X * Multiplier, MouseMoveInt.Y * Multiplier };
    if (Math.Abs(MouseMoveFloat.X) > MaxMouseDelta)
    {
        MouseMoveFloat.X = MaxMouseDelta * Math.Sign(MouseMoveFloat.X);
    }
    if (Math.Abs(MouseMoveFloat.Y) > MaxMouseDelta)
    {
        MouseMoveFloat.Y = MaxMouseDelta * Math.Sign(MouseMoveFloat.Y);
    }

    Camera->Direction.Y += MouseMoveFloat.X * 0.5f; // Yaw = X

    const f32 PitchDirectionDelta = MouseMoveFloat.Y * 0.4f; // Pitch = Y
    Camera->Direction.X += PitchDirectionDelta;

    if (PitchDirectionDelta < 0.0f)
    {
        if ((Camera->Direction.X < -90.0f) || (Camera->Direction.X > 90.0f && Camera->Direction.X < 270.0f))
        {
            Camera->Direction.X = 270.0f;
        }
    }
    else if (PitchDirectionDelta > 0.0f)
    {
        if (Camera->Direction.X > 90.0f && Camera->Direction.X < 270.0f)
        {
            Camera->Direction.X = 90.0f;
        }
    }

    MouseMoveAccum = { 0, 0 };

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
    return Engine.Run<Game::GRaidScene>(Argc, Argv);
}