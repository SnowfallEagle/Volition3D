#include "Engine/Core/Engine.h"
#include "Common/Math/Matrix.h"

using namespace Volition;

namespace Game
{

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

class GAgentWithBladeScene : public GGameState
{
public:
    using Super = GGameState;

private:
    VLight* Spotlight;
    VLight* Spotlight2;
    VLight* CornerLight;

protected:
    virtual void StartUp() override
    {
        Super::StartUp();

        Config.RenderSpec.PostProcessColorCorrection = { 0.6f, 0.6f, 1.0f };

        PointLight->bActive = false;
        SunLight->Color.R -= 0x11;
        SunLight->Color.G -= 0x11;

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

        Renderer.DrawDebugText("Ambient Light [Y]: %s", AmbientLight->bActive ? "On" : "Off");
        Renderer.DrawDebugText("Sun Light     [U]: %s", SunLight->bActive ? "On" : "Off");
        Renderer.DrawDebugText("Spotlight1    [I]: %s", Spotlight->bActive ? "On" : "Off");
        Renderer.DrawDebugText("Spotlight2    [O]: %s", Spotlight2->bActive ? "On" : "Off");
        Renderer.DrawDebugText("Spotlight3    [P]: %s", CornerLight->bActive ? "On" : "Off");
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

        World.SetYShadowPosition(-6000.0f);
        World.GenerateTerrain("Assets/Terrains/Raid/Heightmap.bmp", "Assets/Terrains/RockyLand/Texture.bmp", 500000.0f, 50000.0f, EShadeMode::Gouraud);
        StartSunLightPosition = {1000000.0f, 1000000.0f, 1000000.0f};

        for (i32f i = 0; i < MaxTroopers; ++i)
        {
            Troopers[i] = World.SpawnEntity<VEntity>();
            Troopers[i]->Mesh->LoadMD2("Assets/Models/bobafett/tris.md2", "Assets/Models/bobafett/rotj_fett.pcx", 0, {-11000.0f + 500.0f * (f32)(i % 4), -6000.0f, -17500.0f - ((f32)(i/4) * 1000.0f)}, {20.0f, 20.0f, 20.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
            Troopers[i]->Mesh->PlayAnimation(EMD2AnimationId::Run, true);
        }

        Airplane[0] = World.SpawnEntity<VEntity>();
        Airplane[0]->Mesh->LoadMD2("Assets/Models/viper/tris.md2", nullptr, 0, { -5000.0f, 5000.0f, -40000 }, {100.0f, 100.0f, 100.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Airplane[0]->Mesh->Rotation.X = 30.0f;

        Airplane[1] = World.SpawnEntity<VEntity>();
        Airplane[1]->Mesh->LoadMD2("Assets/Models/viper/tris.md2", nullptr, 0, { -20000.0f, 5000.0f, -40000 }, { 100.0f, 100.0f, 100.0f }, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Airplane[1]->Mesh->Rotation.X = 30.0f;

        Airplane[2] = World.SpawnEntity<VEntity>();
        Airplane[2]->Mesh->LoadMD2("Assets/Models/strogg1/tris.md2", nullptr, 0, { -15000.0f, -5000.0f, -25000 }, {20.0f, 20.0f, 20.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Airplane[2]->Mesh->Rotation.X = 15.0f;

        Airplane[3] = World.SpawnEntity<VEntity>();
        Airplane[3]->Mesh->LoadMD2("Assets/Models/strogg1/tris.md2", nullptr, 0, { -7500.0f, -5000.0f, -25000 }, {20.0f, 20.0f, 20.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
        Airplane[3]->Mesh->Rotation.X = 15.0f;

        World.GetCamera()->Init(ECameraAttr::Euler, {-10000.0f, -5850.0f, -10000.0f}, {-15.0f, 180.0f, 0.0f}, VVector4(), 90.0f, 250.0f, 1000000.0f);

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

class GLargeTerrainScene : public GGameState
{
public:
    using Super = GGameState;

protected:
    virtual void StartUp() override
    {
        Super::StartUp();

        VCamera* Camera = World.GetCamera();
        Camera->Position = { 0.0f, 0.0f, 400000.0f };
        Camera->Direction = { 15.0f, 180.0f, 0.0f };
        Camera->ZFarClip = 3000000.0f;

        CamPosSpeedModifier = 25.0f;

        World.GenerateTerrain("Assets/Terrains/Large/Heightmap.bmp", "Assets/Terrains/RockyLand/Texture.bmp", 1000000.0f, 250000.0f, EShadeMode::Gouraud);
        World.SetYShadowPosition(-102500.0f);
        StartSunLightPosition = { -500000.0f, 1000000.0f, -1000000.0f };

        SunLight->Color.R -= 0x22;
        SunLight->Color.G -= 0x22;
        SunLight->Color.B -= 0x11;

        const auto Boss = World.SpawnEntity()->Mesh->LoadMD2("Assets/Models/boss3/tris.md2", "Assets/Models/boss3/rider.pcx", 0, {-150000.0f, -102500.0f, 225000.0f}, {500.0f, 500.0f, 500.0f}, EShadeMode::Gouraud, {1.5f, 2.0f, 1.5f});
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

    if (Input.IsEventKeyDown(EKeycode::Backspace)) Config.RenderSpec.bRenderSolid ^= true;

    if (Input.IsEventKeyDown(EKeycode::F1)) World.ChangeState<GAgentWithBladeScene>();
    if (Input.IsEventKeyDown(EKeycode::F2)) World.ChangeState<GRaidScene>();
    if (Input.IsEventKeyDown(EKeycode::F3)) World.ChangeState<GLargeTerrainScene>();
    if (Input.IsEventKeyDown(EKeycode::F4)) World.ChangeState<GGrandTerrainScene>();

    if (Input.IsEventKeyDown(EKeycode::F12)) Config.RenderSpec.PostProcessColorCorrection = { 1.0f, 1.0f, 1.0f };
    if (Input.IsEventKeyDown(EKeycode::F5)) Config.RenderSpec.PostProcessColorCorrection = { 1.25f, 1.1f, 1.0f };
    if (Input.IsEventKeyDown(EKeycode::F6)) Config.RenderSpec.PostProcessColorCorrection = { 1.25f, 1.25f, 1.0f };
    if (Input.IsEventKeyDown(EKeycode::F7)) Config.RenderSpec.PostProcessColorCorrection = { 1.0f, 1.25f, 1.0f };
    if (Input.IsEventKeyDown(EKeycode::F8)) Config.RenderSpec.PostProcessColorCorrection = { 1.25f, 1.0f, 1.25f };
    if (Input.IsEventKeyDown(EKeycode::F9)) Config.RenderSpec.PostProcessColorCorrection = { 1.0f, 1.0f, 1.25f };

    if (Input.IsEventKeyDown(EKeycode::Escape)) Engine.Stop();
}

}

int main(int Argc, char** Argv)
{
    return Engine.Run<Game::GRaidScene>(Argc, Argv);
}