#include "Engine/Core/Engine.h"
#include "Common/Math/Matrix.h"

using namespace Volition;

namespace Game
{

class GGameState : public VGameState
{
    VEntity* Entity = nullptr;
    VCamera* Camera;

public:
    virtual void StartUp() override
    {
        /*
        for (i32f i = 0; i < 4; ++i)
        {
            VEntity* TempEntity = World.SpawnEntity<VEntity>();
            TempEntity->Mesh->LoadCOB(
                I == 0 ? "hammer03.cob" :
                    I == 1 ? "jetski05.cob" :
                        I == 2 ? "s.cob" :
                            "tie04.cob",
                { 500.0f * I, 100.0f * Math.Sin(I * 20.0f), 250.0f},
                { 100.0f, 100.0f, 100.0f },
                ECOB::SwapYZ | ECOB::InvertV
            );
        }
        */

        Entity = World.SpawnEntity<VEntity>();
        Entity->Mesh->LoadCOB(
            "Assets/Models/jetski05.cob",
            { 0.0f, 0.0f, 0.0f },
            { 1000.0f, 1000.0f, 1000.0f },
            ECOB::SwapYZ | ECOB::InvertV
        );
        Entity->Mesh->Attr &= ~EMeshAttr::CastShadow;

        Camera = World.GetCamera();
        Camera->Init(ECameraAttr::Euler, { 0, 75.0f, 0 }, { 0, 0, 0 }, VVector4(), 90, 500, 100000, {(f32)Renderer.GetScreenWidth(), (f32)Renderer.GetScreenHeight()});

        World.SetCubemap("Assets/Cubemaps/Cubemap.png");
        World.GetTerrain()->GenerateTerrain("Assets/Terrains/Large/Heightmap.bmp", "Assets/Terrains/Common/Texture.bmp", 25000.0f, 10000.0f);

        {
            VLight AmbientLight = {
                ELightState::Active,
                ELightAttr::Ambient,

                MAP_XRGB32(0x33, 0x33, 0x22), 0, 0,
                { 0, 0, 0, 0 }, { 0, 0, 0, 0}, VVector4{0, 0, 0, 0}.GetNormalized(), { 0 , 0, 0, 0 },

                0, 0, 0,
                0, 0,
                0
            };

            VLight InfiniteLight = {
                ELightState::Active,
                ELightAttr::Infinite,

                0, MAP_XRGB32(0x77, 0x66, 0x22), 0,
                { 5000, 5000, 5000, 0 }, { 0, 0, 0, 0 }, VVector4{ -1.0f, -1.0f, 0, 0 }.GetNormalized(), { 0, 0, 0, 0 },

                0, 0, 0,
                0, 0,
                0
            };

            VLight PointLight = {
                ELightState::Active,
                ELightAttr::Point,

                0, MAP_XRGB32(0x33, 0x00, 0x77), 0,
                { 1000.0f, 1000.0f, 0.0f, 0 }, { 0, 0, 0, 0 }, VVector4{ 0, 0, 0, 0 }.GetNormalized(), { 0, 0, 0, 0 },

                0, 0.001f, 0,
                0, 0,
                0
            };

            VLight OccluderLight = {
                ELightState::Active,
                ELightAttr::Point,

                0, MAP_XRGB32(0x00, 0x00, 0xAA), 0,
                { 0.0f, 1000.0f, 1000.0f, 0 }, { 0, 0, 0, 0 }, VVector4{ 0, 0, 0, 0 }.GetNormalized(), { 0, 0, 0, 0 },

                0, 0.0001f, 0,
                0, 0,
                0
            };

            VLight SimpleSpotlight = {
                ELightState::Active,
                ELightAttr::SimpleSpotlight,

                0, MAP_XRGB32(0xAA, 0xAA, 0xAA), 0,
                { 1000.0f, 1000.0f, 0.0f, 0 }, { 0, 0, 0, 0 }, VVector4(-1.0f, -1.0f, 0.0f).GetNormalized(), { 0, 0, 0, 0 },

                0, 0.0005f, 0,
                30.0f, 60.0f,
                1.0f
            };

            VLight ComplexSpotlight = {
                ELightState::Active,
                ELightAttr::ComplexSpotlight,

                0, MAP_XRGB32(0xBB, 0x00, 0x00), 0,
                { 0.0f, 1000.0f, -300.0f, 0 }, { 0, 0, 0, 0 }, VVector4(-0.5f, -1.0f, 1.0f).GetNormalized(), { 0, 0, 0, 0 },

                0, 0.0005f, 0,
                30.0f, 60.0f,
                1.0f
            };

            Renderer.AddLight(AmbientLight);
            Renderer.AddLight(InfiniteLight);
            // Renderer.AddLight(PointLight);
            Renderer.AddLight(ComplexSpotlight);
            // Renderer.AddLight(OccluderLight);
            Renderer.SetOccluderLight(3);
        }
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

        f32 CamDirSpeed = 0.5f * DeltaTime;
        if (Input.IsKeyDown(EKeycode::Left))  Camera->Dir.Y -= CamDirSpeed;
        if (Input.IsKeyDown(EKeycode::Right)) Camera->Dir.Y += CamDirSpeed;
        if (Input.IsKeyDown(EKeycode::Up))    Camera->Dir.X -= CamDirSpeed;
        if (Input.IsKeyDown(EKeycode::Down))  Camera->Dir.X += CamDirSpeed;

        if (Input.IsKeyDown(EKeycode::Space)) Camera->Pos.Y += CamDirSpeed * ShiftModifier;
        if (Input.IsKeyDown(EKeycode::C))     Camera->Pos.Y -= CamDirSpeed * ShiftModifier;

        VMatrix44 Rot = VMatrix44::Identity;
        f32 Speed = 0.05f * DeltaTime;
        if (Input.IsKeyDown(EKeycode::Q)) Rot.BuildRotationXYZ(0, Speed, 0);
        if (Input.IsKeyDown(EKeycode::E)) Rot.BuildRotationXYZ(0, -Speed, 0);
        if (Input.IsKeyDown(EKeycode::F)) Rot.BuildRotationXYZ(0, 0, Speed);
        if (Input.IsKeyDown(EKeycode::G)) Rot.BuildRotationXYZ(0, 0, -Speed);
        if (Input.IsKeyDown(EKeycode::Z)) Rot.BuildRotationXYZ(Speed, 0, 0);
        if (Input.IsKeyDown(EKeycode::X)) Rot.BuildRotationXYZ(-Speed, 0, 0);

        if (Entity)
        {
            Entity->Mesh->Transform(Rot, ETransformType::LocalOnly, true);
        }

        if (Input.IsKeyDown(EKeycode::Backspace))
        {
            Config.RenderSpec.bRenderSolid ^= true;
        }

        Renderer.DrawDebugText("FPS: %.2f", 1000.0f / DeltaTime);
    }
};

}

int main(int Argc, char** Argv)
{
    return Engine.Run<Game::GGameState>(Argc, Argv);
}