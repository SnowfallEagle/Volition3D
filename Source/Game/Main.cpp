#include "Engine/Core/Engine.h"
#include "Common/Math/Matrix.h"

using namespace Volition;

namespace Game
{

class GCycleAnimatedEntity : public VEntity
{
private:
    using Super = VEntity;

private:
    i32 AnimationId;

public:
    void StartAnimationsCycle()
    {
        AnimationId = 0;
        Mesh->PlayAnimation((EMD2AnimationId)AnimationId);
    }

    virtual void Update(f32 DeltaTime) override
    {
        Super::Update(DeltaTime);

        if (Mesh->bAnimationPlayed)
        {
            AnimationId = (AnimationId + 1) % (i32)EMD2AnimationId::MaxAnimations;
            Mesh->PlayAnimation((EMD2AnimationId)AnimationId);
        }
    }
};

class GGameState : public VGameState
{
protected:
    VString StateName = "Untitled";

    VVector2 MouseMoveAccum = { 0.0f, 0.0f };

    f32 ShiftCamPosSpeedModifier = 5.0f;
    f32 CamPosSpeedModifier      = 2.5f;
    f32 CamDirSpeedModifier      = 0.5f;

    f32 MouseSensivity = 0.0475f;
    f32 MaxMouseDelta  = 30.0f;

    VVector4 StartSunLightPosition;

    VLight* AmbientLight;
    VLight* PointLight;
    VLight* SunLight;

protected:
    virtual void StartUp() override
    {
        World.SetEnvironment2D("Assets/Environment2D/Texture.png");

        AmbientLight = World.SpawnLight(ELightType::Ambient);
        PointLight = World.SpawnLight(ELightType::Point);

        SunLight = World.SpawnLight(ELightType::Infinite);
        StartSunLightPosition = SunLight->Position;

        World.SetShadowMakingLight(SunLight);
        World.Environment2DMovementEffectAngle = 165.0f;

        Config.RenderSpec.PostProcessColorCorrection = { 1.25f, 1.1f, 1.0f };
    }

    virtual void Update(f32 DeltaTime) override
    {
        ProcessInput(DeltaTime);

        Renderer.DrawDebugText("Scene: %s", *StateName);

        Config.RenderSpec.DebugTextPosition.Y += Renderer.GetFontCharHeight();

        Renderer.DrawDebugText("Controls:", Config.RenderSpec.bRenderSolid ? "Solid" : "Wire");
        Renderer.DrawDebugText("  Render [Backspace]: %s", Config.RenderSpec.bRenderSolid ? "Solid" : "Wire");
        Renderer.DrawDebugText("  Choose Scene      [F1-F5]");
        Renderer.DrawDebugText("  Scale Target Size [1-3]");
        Renderer.DrawDebugText("  Color Correction  [F7-F12]");
        Renderer.DrawDebugText("  Toggle UI         [Tab]");

        if (Input.IsEventKeyDown(EKeycode::F7))  Config.RenderSpec.PostProcessColorCorrection = { 1.0f, 1.0f, 1.0f };
        if (Input.IsEventKeyDown(EKeycode::F8))  Config.RenderSpec.PostProcessColorCorrection = { 1.25f, 1.1f, 1.0f };
        if (Input.IsEventKeyDown(EKeycode::F9))  Config.RenderSpec.PostProcessColorCorrection = { 1.25f, 1.25f, 1.0f };
        if (Input.IsEventKeyDown(EKeycode::F10)) Config.RenderSpec.PostProcessColorCorrection = { 1.0f, 1.25f, 1.0f };
        if (Input.IsEventKeyDown(EKeycode::F11)) Config.RenderSpec.PostProcessColorCorrection = { 1.25f, 1.0f, 1.25f };
        if (Input.IsEventKeyDown(EKeycode::F12)) Config.RenderSpec.PostProcessColorCorrection = { 1.0f, 1.0f, 1.25f };

        if (Input.IsKeyDown(EKeycode::N1)) Config.RenderSpec.RenderScale = VLN_MAX(Config.RenderSpec.RenderScale - 0.001f * DeltaTime, 0.1f);
        if (Input.IsKeyDown(EKeycode::N2)) Config.RenderSpec.RenderScale = VLN_MIN(Config.RenderSpec.RenderScale + 0.001f * DeltaTime, 2.0f);
        if (Input.IsKeyDown(EKeycode::N3)) Config.RenderSpec.RenderScale = 1.0f;

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

class GThreatScene : public GGameState
{
private:
    using Super = GGameState;

private:
    VLight* Spotlight;
    VLight* Spotlight2;
    VLight* CornerLight;

protected:
    virtual void StartUp() override
    {
        Super::StartUp();

        StateName = "Threat";

        Config.RenderSpec.PostProcessColorCorrection = { 0.6f, 0.6f, 1.0f };

        PointLight->bActive = false;
        SunLight->Color.R -= 0x33;
        SunLight->Color.G -= 0x22;

        Spotlight = World.SpawnLight(ELightType::ComplexSpotlight);
        Spotlight->Position = { 0.0f, -10000.0f, 250.0f };
        Spotlight->Direction = VVector4{ 0.0f, -0.75f, -0.5f }.GetNormalized();
        Spotlight->Color = MAP_XRGB32(0x88, 0x88, 0xAA);
        Spotlight->KQuad = 0.0000001f;
        Spotlight->KLinear = 0.0f;
        Spotlight->FalloffPower = 5.0f;

        Spotlight2 = World.SpawnLight(ELightType::ComplexSpotlight);
        Spotlight2->Position = { 0.0f, -10000.0f, -250.0f };
        Spotlight2->Direction = VVector4{ 0.0f, -0.5f, 1.0f }.GetNormalized();
        Spotlight2->Color = MAP_XRGB32(0xAA, 0x22, 0x22);
        Spotlight2->KQuad = 0.00000005f;
        Spotlight2->KLinear = 0.0f;
        Spotlight2->FalloffPower = 5.0f;

        CornerLight = World.SpawnLight(ELightType::ComplexSpotlight);
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
        Trooper2->Mesh->LoadMD2("Assets/Models/Marine/tris.md2", "Assets/Models/Marine/Reese.pcx", 0, {1000.0f, -12200.0f, 4000.0f}, { 20.0f, 20.0f, 20.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Trooper2->Mesh->PlayAnimation(EMD2AnimationId::StandingIdle, true);
        Trooper2->Mesh->Rotation = { 0.0f, 210.0f, 0.0f };

        const auto Trooper3 = World.SpawnEntity<VEntity>();
        Trooper3->Mesh->LoadMD2("Assets/Models/Marine/tris.md2", "Assets/Models/Marine/Reese.pcx", 0, {-1000.0f, -12200.0f, 4000.0f}, { 20.0f, 20.0f, 20.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Trooper3->Mesh->PlayAnimation(EMD2AnimationId::StandingIdle, true);
        Trooper3->Mesh->AnimationTimeAccum += 1.0f;
        Trooper3->Mesh->Rotation = { 0.0f, 150.0f, 0.0f };

        const auto Trooper4 = World.SpawnEntity<VEntity>();
        Trooper4->Mesh->LoadMD2("Assets/Models/Marine/tris.md2", "Assets/Models/Marine/USMC.pcx", 0, {-8000.0f, -12200.0f, -15000.0f}, { 20.0f, 20.0f, 20.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Trooper4->Mesh->PlayAnimation(EMD2AnimationId::Wave, true);
        Trooper4->Mesh->Rotation = { 0.0f, 30.0f, 0.0f };

        const auto Trooper5 = World.SpawnEntity<VEntity>();
        Trooper5->Mesh->LoadMD2("Assets/Models/Marine/tris.md2", "Assets/Models/Marine/Brownie.pcx", 0, {-6000.0f, -12200.0f, -15000.0f}, { 20.0f, 20.0f, 20.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Trooper5->Mesh->PlayAnimation(EMD2AnimationId::Salute, true);
        Trooper5->Mesh->Rotation = { 0.0f, 00.0f, 0.0f };
        Trooper5->Mesh->AnimationTimeAccum += 0.4f;

        const auto Blade = World.SpawnEntity<VEntity>();
        Blade->Mesh->LoadMD2("Assets/Models/blade/tris.md2", "Assets/Models/blade/blade.pcx", 0, {0.0f, -12200.0f, 0.0f}, {20.0f, 20.0f, 20.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Blade->Mesh->PlayAnimation(EMD2AnimationId::StandingIdle, true);

        const auto Dead = World.SpawnEntity<VEntity>();
        Dead->Mesh->LoadMD2("Assets/Models/deadbods/dude/tris.md2", "Assets/Models/deadbods/dude/dead1.pcx", 0, { 5000.0f, -12200.0f, 4750.0f }, { 15.0f, 15.0f, 15.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Dead->Mesh->Rotation.Y = 90.0f;

        World.GetCamera()->Init(ECameraAttr::Euler, {7000.0f, -11000.0f, 5500.0f}, {5.0f, -135.0f, 0.0f}, VVector4(), 90.0f, 75.0f, 1000000.0f);

        CamPosSpeedModifier *= 0.25f;
    }

    virtual void Update(f32 DeltaTime) override
    {
        Super::Update(DeltaTime);

        Config.RenderSpec.DebugTextPosition.Y += Renderer.GetFontCharHeight();

        Renderer.DrawDebugText("Lights:");
        Renderer.DrawDebugText("  Ambient Light [Y]: %s", AmbientLight->bActive ? "On" : "Off");
        Renderer.DrawDebugText("  Sun Light     [U]: %s", SunLight->bActive ? "On" : "Off");
        Renderer.DrawDebugText("  Spotlight1    [I]: %s", Spotlight->bActive ? "On" : "Off");
        Renderer.DrawDebugText("  Spotlight2    [O]: %s", Spotlight2->bActive ? "On" : "Off");
        Renderer.DrawDebugText("  Spotlight3    [P]: %s", CornerLight->bActive ? "On" : "Off");
    }

    virtual void ProcessInput(f32 DeltaTime) override
    {
        Super::ProcessInput(DeltaTime);

        if (Input.IsEventKeyDown(EKeycode::Y)) AmbientLight->bActive ^= true;
        if (Input.IsEventKeyDown(EKeycode::U)) SunLight->bActive ^= true;
        if (Input.IsEventKeyDown(EKeycode::I)) Spotlight->bActive ^= true;
        if (Input.IsEventKeyDown(EKeycode::O)) Spotlight2->bActive ^= true;
        if (Input.IsEventKeyDown(EKeycode::P)) CornerLight->bActive ^= true;
    }
};

class GRaidScene : public GGameState
{
private:
    enum class ERaidState
    {
        Run = 0,
        Stop
    };

    using Super = GGameState;

private:
    static constexpr i32f MaxTroopers = 48;
    static constexpr i32f MaxAirplanes = 4;

private:
    VEntity* Troopers[MaxTroopers];
    VEntity* Airplane[MaxAirplanes];

    ERaidState RaidState;
    f32 RunTimer;

protected:
    virtual void StartUp() override
    {
        Super::StartUp();

        StateName = "Raid";

        Config.RenderSpec.PostProcessColorCorrection = { 1.1f, 1.05f, 1.0f };

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

        AmbientLight->Color = MAP_XRGB32(0x33, 0x22, 0x11);

        World.Environment2DMovementEffectSpeed *= 1.0f;
        World.SetEnvironment2D("Assets/Environment2D/Afternoon.png");
        World.GenerateTerrain("Assets/Terrains/Raid/Heightmap.bmp", "Assets/Terrains/RockyLand/Texture.bmp", 500000.0f, 50000.0f, EShadeMode::Gouraud);
        World.SetYShadowPosition(-6000.0f);
        StartSunLightPosition = {1000000.0f, 1000000.0f, 1000000.0f};

        for (i32f i = 0; i < MaxTroopers; ++i)
        {
            static constexpr const char* TexturesPath[] = {
                "Assets/Models/bobafett/rotj_fett.pcx",
                "Assets/Models/bobafett/prototype_fett.pcx",
                "Assets/Models/bobafett/esb_fett.pcx",
                "Assets/Models/bobafett/prototype_fett.pcx",
                "Assets/Models/bobafett/esb_fett.pcx",
                "Assets/Models/bobafett/esb_fett.pcx",
                "Assets/Models/bobafett/prototype_fett.pcx",
                "Assets/Models/bobafett/prototype_fett.pcx",
                "Assets/Models/bobafett/esb_fett.pcx",
                "Assets/Models/bobafett/esb_fett.pcx",
                "Assets/Models/bobafett/prototype_fett.pcx",
            };

            Troopers[i] = World.SpawnEntity<VEntity>();
            Troopers[i]->Mesh->LoadMD2("Assets/Models/bobafett/tris.md2", TexturesPath[(i / 4) % VLN_ARRAY_SIZE(TexturesPath)], 0, {-16000.0f + 500.0f * (f32)(i % 4), -6000.0f, -17500.0f - ((f32)(i / 4) * 1000.0f)}, {20.0f, 20.0f, 20.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
            Troopers[i]->Mesh->PlayAnimation(EMD2AnimationId::Run, true);
        }

        Airplane[0] = World.SpawnEntity<VEntity>();
        Airplane[0]->Mesh->LoadMD2("Assets/Models/viper/tris.md2", nullptr, 0, { -10000.0f, 5000.0f, -40000 }, {100.0f, 100.0f, 100.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Airplane[0]->Mesh->Rotation.X = 30.0f;

        Airplane[1] = World.SpawnEntity<VEntity>();
        Airplane[1]->Mesh->LoadMD2("Assets/Models/viper/tris.md2", nullptr, 0, { -25000.0f, 5000.0f, -40000 }, { 100.0f, 100.0f, 100.0f }, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Airplane[1]->Mesh->Rotation.X = 30.0f;

        Airplane[2] = World.SpawnEntity<VEntity>();
        Airplane[2]->Mesh->LoadMD2("Assets/Models/strogg1/tris.md2", nullptr, 0, { -20000.0f, -5000.0f, -25000 }, {20.0f, 20.0f, 20.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Airplane[2]->Mesh->Rotation.X = 15.0f;

        Airplane[3] = World.SpawnEntity<VEntity>();
        Airplane[3]->Mesh->LoadMD2("Assets/Models/strogg1/tris.md2", nullptr, 0, { -12500.0f, -5000.0f, -25000 }, {20.0f, 20.0f, 20.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Airplane[3]->Mesh->Rotation.X = 15.0f;

        World.GetCamera()->Init(ECameraAttr::Euler, {-15000.0f, -5850.0f, -10000.0f}, {-15.0f, 180.0f, 0.0f}, VVector4(), 90.0f, 250.0f, 1000000.0f);

        RaidState = ERaidState::Run;
        RunTimer = 7500.0f;
    }

    virtual void Update(f32 DeltaTime) override
    {
        Super::Update(DeltaTime);

        if (RaidState == ERaidState::Run)
        {
            for (i32f i = 0; i < MaxTroopers; ++i)
            {
                Troopers[i]->Mesh->Position.Z += 2.0f * DeltaTime;
            }

            for (i32f i = 0; i < MaxAirplanes; ++i)
            {
                Airplane[i]->Mesh->Position.Z += (i < 2 ? 1.0f : 2.0f) * DeltaTime;
            }

            RunTimer -= DeltaTime;
            if (RunTimer < 0.0f)
            {
                for (i32f i = 0; i < MaxTroopers; ++i)
                {
                    Troopers[i]->Mesh->PlayAnimation(EMD2AnimationId::CrouchStand, true);
                    Troopers[i]->Mesh->AnimationTimeAccum += (f32)Math.Random(0, 1000) / 1000.0f;
                }

                RaidState = ERaidState::Stop;
            }
        }
    }
};

class GBigGuyScene : public GGameState
{
private:
    using Super = GGameState;

protected:
    virtual void StartUp() override
    {
        Super::StartUp();

        StateName = "Big Guy";

        VCamera* Camera = World.GetCamera();
        Camera->Position = { 10750.0f, 27500.0f, 385000.0f };
        Camera->Direction = { 15.0f, -157.5f, 0.0f };
        Camera->ZFarClip = 3000000.0f;

        CamPosSpeedModifier = 25.0f;

        World.SetEnvironment2D("Assets/Environment2D/Morning.png");
        World.GenerateTerrain("Assets/Terrains/Large/Heightmap.bmp", "Assets/Terrains/RockyLand/Texture.bmp", 1000000.0f, 250000.0f, EShadeMode::Gouraud);
        World.SetYShadowPosition(-102500.0f);
        StartSunLightPosition = { -500000.0f, 1000000.0f, -1000000.0f };

        AmbientLight->Color = MAP_XRGB32(0x10, 0x0C, 0x04);
        SunLight->Color.R -= 0x11;
        SunLight->Color.G -= 0x11;
        SunLight->Color.B -= 0x22;
        PointLight->bActive = false;

        const auto Boss = World.SpawnEntity<GCycleAnimatedEntity>();
        Boss->Mesh->LoadMD2("Assets/Models/boss3/tris.md2", "Assets/Models/boss3/rider.pcx", 0, {-150000.0f, -102500.0f, 225000.0f}, {500.0f, 500.0f, 500.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Boss->StartAnimationsCycle();
    }
};

class GGrandTerrainScene : public GGameState
{
private:
    using Super = GGameState;

protected:
    virtual void StartUp() override
    {
        Super::StartUp();

        StateName = "Grand Terrain";

        AmbientLight->Color = MAP_XRGB32(0x09, 0x06, 0x03);
        SunLight->Color = MAP_XRGB32(0x88, 0x77, 0x33);
        PointLight->bActive = false;

        VCamera* Camera = World.GetCamera();
        Camera->Position = { 350000.0f, 0.0f, -750000.0f };
        Camera->Direction = { 35.0f, -35.0f, 0.0f };
        Camera->ZFarClip = 5000000.0f;

        CamPosSpeedModifier = 25.0f;
        CamDirSpeedModifier = 0.05f;

        World.Environment2DMovementEffectSpeed = 0.0f;
        World.SetEnvironment2D("Assets/Environment2D/Land.png");
        World.GenerateTerrain("Assets/Terrains/Grand/Heightmap.png", "Assets/Terrains/Grand/Texture.png", 2000000.0f, 2000000.0f, EShadeMode::Gouraud);
    }
};

class GShadowsAndLightsScene : public GGameState
{
private:
    using Super = GGameState;

private:
    static constexpr f32 LightStates[]    = { 5000.0f, 3000.0f, 1500.0f, 3000.0f, 2000.0f, 500.0f, 4000.0f, 1000.0f, 1000.0f, 1500.0, 5000.0f };
    static constexpr f32 LightIntensities[] = { 1.0f, 1.1f, 1.0f, 1.1f, 1.0f, 0.5f, 1.0f, 1.1f, 1.0f, 1.1f, 1.0f };
    
    static constexpr VColorARGB Light1DebugTextColor = MAP_XRGB32(0xDD, 0x22, 0x66);
    static constexpr VColorARGB Light2DebugTextColor = MAP_XRGB32(0x66, 0x22, 0xDD);

private:
    VLight* Light1;
    VLight* Light2;

    VVector4 Light1StartPosition;
    VVector4 Light2StartPosition;
    VVector4 LightStartDirection;
    
    f32 RotationTimeAccum;
    f32 Light1TimeAccum;
    f32 Light2TimeAccum;

    VColorARGB Light1OriginalColor;
    VColorARGB Light2OriginalColor;

    i32 Light1State;
    i32 Light2State;

protected:
    virtual void StartUp() override
    {
        Super::StartUp();

        StateName = "Shadows And Lights";

        Config.RenderSpec.PostProcessColorCorrection = { 1.1f, 0.9f, 0.9f };

        VCamera* Camera = World.GetCamera();
        Camera->Position = { 0.0f, 7500.0f, 25000.0f };
        Camera->Direction = { 15.0f, 180.0f, 0.0f };

        CamPosSpeedModifier = 25.0f;

        World.SetEnvironment2D("Assets/Environment2D/Night.png");
        World.Environment2DMovementEffectSpeed = 0.0f;
        World.GenerateTerrain("Assets/Terrains/Small/SmallHeightmap.bmp", "Assets/Terrains/Common/Blue.bmp", 100000.0f, 2500.0f, EShadeMode::Gouraud);
        World.SetYShadowPosition(-900.0f);

        const auto Entity1 = World.SpawnEntity<GCycleAnimatedEntity>();
        Entity1->Mesh->LoadMD2("Assets/Models/droideka/tris.md2", "Assets/Models/droideka/droideka.pcx", 0, { 5000.0f, -900.0f, 0.0f}, {100.0f, 100.0f, 100.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Entity1->StartAnimationsCycle();

        const auto Entity2 = World.SpawnEntity<GCycleAnimatedEntity>();
        Entity2->Mesh->LoadMD2("Assets/Models/McClane/tris.md2", "Assets/Models/McClane/nakatomi1.pcx", 0, { -6000.0f, -900.0f, 0.0f}, {100.0f, 100.0f, 100.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Entity2->StartAnimationsCycle();

        AmbientLight->Color = MAP_XRGB32(0x0A, 0x0A, 0x0A);

        SunLight->Color = 0;
        StartSunLightPosition = { 15000.0f, 15000.0f, 15000.0f };

        PointLight->bActive = false;

        LightStartDirection = VVector4{ 0.0f, -1.0f, -0.25f }.GetNormalized();

        Light1 = World.SpawnLight(ELightType::ComplexSpotlight);
        Light1StartPosition = { Entity1->Mesh->Position.X, 10000.0f, Entity1->Mesh->Position.Z + 1000.0f };
        Light1->Position = Light1StartPosition;
        Light1->Direction = LightStartDirection;
        Light1OriginalColor = MAP_XRGB32(0x66, 0x0C, 0x00);
        Light1->Color = Light1OriginalColor;
        Light1->KLinear = 0.00001f;
        Light1->FalloffPower = 5.0f;

        Light2 = World.SpawnLight(ELightType::ComplexSpotlight);
        Light2StartPosition = { Entity2->Mesh->Position.X, 10000.0f, Entity2->Mesh->Position.Z + 1000.0f };
        Light2->Position = Light2StartPosition;
        Light2->Direction = LightStartDirection;
        Light2OriginalColor = MAP_XRGB32(0x00, 0x0C, 0x88);
        Light2->Color = Light2OriginalColor;
        Light2->KLinear = 0.00001f;
        Light2->FalloffPower = 5.0f;

        World.SetShadowMakingLight(Light1);

        RotationTimeAccum = 0.0f;
        Light1TimeAccum   = 0.0f;
        Light2TimeAccum   = 0.0f;

        Light1State = 0;
        Light2State = VLN_ARRAY_SIZE(LightStates) - 1;
    }

    virtual void Update(f32 DeltaTime) override
    {
        Super::Update(DeltaTime);

        RotationTimeAccum += DeltaTime;
        Light1TimeAccum   += DeltaTime;
        Light2TimeAccum   += DeltaTime;

        const f32 Angle = RotationTimeAccum / 100.0f;

        VMatrix44 MatTransform;
        MatTransform.BuildRotationXYZ(Math.Cos(Angle) * 30.0f, Math.Sin(Angle) * 360.0f, 0.0f);

        VMatrix44::MulVecMat(Light1StartPosition, MatTransform, Light1->Position);
        VMatrix44::MulVecMat(Light2StartPosition, MatTransform, Light2->Position);

        VMatrix44::MulVecMat(LightStartDirection, MatTransform, Light1->Direction);
        Light2->Direction = Light1->Direction;

        if (Light1TimeAccum > LightStates[Light1State])
        {
            Light1->bActive ^= true;
            Light1TimeAccum = 0.0f;

            Light1State = (Light1State + 1) % VLN_ARRAY_SIZE(LightStates);
            Light1->Color = MAP_XRGB32((i32)VLN_MIN((f32)Light1OriginalColor.R * LightIntensities[Light1State], 255), Light1OriginalColor.G, Light1OriginalColor.B);
        }

        if (Light2TimeAccum > LightStates[Light2State])
        {
            Light2->bActive ^= true;
            Light2TimeAccum = 0.0f;

            Light2State = (Light2State - 1) % VLN_ARRAY_SIZE(LightStates);
            Light2->Color = MAP_XRGB32(Light2OriginalColor.R, Light2OriginalColor.G, (i32)VLN_MIN((f32)Light2OriginalColor.B * LightIntensities[Light2State], 255));
        }

        Config.RenderSpec.DebugTextPosition.Y += Renderer.GetFontCharHeight();

        Renderer.DrawDebugText("Shadow Making Light:");
        Renderer.DrawDebugText("  Spotlight1 [Y]");
        Renderer.DrawDebugText("  Spotlight2 [U]");
        Renderer.DrawDebugText("  Sun Light  [I]");
    }

    virtual void ProcessInput(f32 DeltaTime) override
    {
        Super::ProcessInput(DeltaTime);

        if (Input.IsEventKeyDown(EKeycode::Y))
        {
            World.SetShadowMakingLight(Light1);
            Config.RenderSpec.DebugTextColor = Light1DebugTextColor;
        }
        if (Input.IsEventKeyDown(EKeycode::U))
        {
            World.SetShadowMakingLight(Light2);
            Config.RenderSpec.DebugTextColor = Light2DebugTextColor;
        }
        if (Input.IsEventKeyDown(EKeycode::I))
        {
            World.SetShadowMakingLight(SunLight);
            Config.RenderSpec.DebugTextColor = Config.RenderSpec.DefaultDebugTextColor;
        }
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

    if (Input.IsEventKeyDown(EKeycode::Backspace)) Config.RenderSpec.bRenderSolid ^= true;
    if (Input.IsEventKeyDown(EKeycode::Tab)) Config.RenderSpec.bRenderUI ^= true;

    if (Input.IsEventKeyDown(EKeycode::F1)) World.ChangeState<GThreatScene>();
    if (Input.IsEventKeyDown(EKeycode::F2)) World.ChangeState<GRaidScene>();
    if (Input.IsEventKeyDown(EKeycode::F3)) World.ChangeState<GBigGuyScene>();
    if (Input.IsEventKeyDown(EKeycode::F4)) World.ChangeState<GGrandTerrainScene>();
    if (Input.IsEventKeyDown(EKeycode::F5)) World.ChangeState<GShadowsAndLightsScene>();

    if (Input.IsEventKeyDown(EKeycode::F7))  Config.RenderSpec.PostProcessColorCorrection = { 1.0f, 1.0f, 1.0f };
    if (Input.IsEventKeyDown(EKeycode::F8))  Config.RenderSpec.PostProcessColorCorrection = { 1.25f, 1.1f, 1.0f };
    if (Input.IsEventKeyDown(EKeycode::F9))  Config.RenderSpec.PostProcessColorCorrection = { 1.25f, 1.25f, 1.0f };
    if (Input.IsEventKeyDown(EKeycode::F10)) Config.RenderSpec.PostProcessColorCorrection = { 1.0f, 1.25f, 1.0f };
    if (Input.IsEventKeyDown(EKeycode::F11)) Config.RenderSpec.PostProcessColorCorrection = { 1.25f, 1.0f, 1.25f };
    if (Input.IsEventKeyDown(EKeycode::F12)) Config.RenderSpec.PostProcessColorCorrection = { 1.0f, 1.0f, 1.25f };

    if (Input.IsKeyDown(EKeycode::N1)) Config.RenderSpec.RenderScale = VLN_MAX(Config.RenderSpec.RenderScale - 0.001f * DeltaTime, 0.1f);
    if (Input.IsKeyDown(EKeycode::N2)) Config.RenderSpec.RenderScale = VLN_MIN(Config.RenderSpec.RenderScale + 0.001f * DeltaTime, 2.0f);
    if (Input.IsKeyDown(EKeycode::N3)) Config.RenderSpec.RenderScale = 1.0f;

    if (Input.IsEventKeyDown(EKeycode::Escape)) Engine.Stop();
}

}

int main(int Argc, char** Argv)
{
    return Engine.Run<Game::GRaidScene>(Argc, Argv);
}