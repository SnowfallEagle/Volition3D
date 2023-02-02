/* NOTE(sean): It should be file main file for game that uses engine */

#include "Engine/Core/Volition.h"
#include "Engine/Math/Matrix.h"

class GMyGameFlow : public VGameFlow
{
    VObject* Object;
    VCamera* Camera;

public:
    virtual void StartUp() override
    {
        for (i32f I = 0; I < 3; ++I)
        {
            VObject* TempObject = World.SpawnObject<VObject>();
            TempObject->LoadCOB(
                "rec_gouraud_textured_02.cob",
                { 500.0f * I, 100.0f * Math.Sin(I * 20.0f), 250.0f},
                { 100.0f, 100.0f, 100.0f },
                { 0.0f, 0.0f, 0.0f }
            );
        }

        Object = World.SpawnObject<VObject>();
        Object->LoadCOB(
            "rec_gouraud_textured_02.cob",
            { 0.0f, 0.0f, 250.0f },
            { 100.0f, 100.0f, 100.0f },
            { 0.0f, 0.0f, 0.0f }
        );

        // TODO(sean): Later we should spawn camera as entity and attach it to world
        Camera = World.GetCamera();
        Camera->Init(ECameraAttr::Euler, { 0, 75.0f, 0 }, { 0, 0, 0 }, Object->Position, 90, 50, 12000, { (f32)Renderer.GetScreenWidth(), (f32)Renderer.GetScreenHeight()});

        {
            VLight AmbientLight = {
                0,
                ELightState::Active,
                ELightAttr::Ambient,

                MAP_RGBX32(0x33, 0x33, 0x33), 0, 0,
                { 0, 0, 0, 0 }, { 0, 0, 0, 0}, VVector4{0, 0, 0, 0}.GetNormalized(), { 0 , 0, 0, 0 },

                0, 0, 0,
                0, 0,
                0
            };

            VLight InfiniteLight = {
                1,
                ELightState::Active,
                ELightAttr::Infinite,

                0, MAP_RGBX32(0x88, 0x44, 0x22), 0,
                { 0, 1000, 1000, 0 }, { 0, 0, 0, 0 }, VVector4{ -1.0f, -1.0f, 0, 0 }.GetNormalized(), { 0, 0, 0, 0 },

                0, 0, 0,
                0, 0,
                0
            };

            VLight PointLight = {
                2,
                ELightState::Active,
                ELightAttr::Point,

                0, MAP_RGBX32(0xFF, 0xFF, 0xFF), 0,
                { 1000.0f, 1000.0f, 0, 0 }, { 0, 0, 0, 0 }, VVector4{ 0, 0, 0, 0 }.GetNormalized(), { 0, 0, 0, 0 },

                0, 0.0001f, 0,
                0, 0,
                0
            };

            VLight SimpleSpotlight = {
                3,
                ELightState::Active,
                ELightAttr::SimpleSpotlight,

                0, MAP_RGBX32(0xAA, 0xAA, 0xAA), 0,
                { 1000.0f, 1000.0f, 0.0f, 0 }, { 0, 0, 0, 0 }, VVector4(-1.0f, -1.0f, 0.0f).GetNormalized(), { 0, 0, 0, 0 },

                0, 0.0005f, 0,
                30.0f, 60.0f,
                1.0f
            };

            VLight ComplexSpotlight = {
                3,
                ELightState::Active,
                ELightAttr::ComplexSpotlight,

                0, MAP_RGBX32(0xCC, 0xCC, 0xCC), 0,
                { 0.0f, 1000.0f, -300.0f, 0 }, { 0, 0, 0, 0 }, VVector4(-0.5f, -1.0f, 1.0f).GetNormalized(), { 0, 0, 0, 0 },

                0, 0.0005f, 0,
                30.0f, 60.0f,
                1.0f
            };

            Renderer.AddLight(AmbientLight);
            //Renderer.AddLight(InfiniteLight);
            //Renderer.AddLight(PointLight);
            Renderer.AddLight(ComplexSpotlight);
            //Renderer.AddLight(SimpleSpotlight);
        }
    }

    virtual void Update(f32 DeltaTime) override
    {
        if (Input.IsKeyDown(EKeycode::Escape))
        {
            Volition.Stop();
        }

        f32 CamPosSpeed = 0.1f * DeltaTime;
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

        f32 CamDirSpeed = 0.2f * DeltaTime;
        if (Input.IsKeyDown(EKeycode::Left))
        {
            Camera->Dir.Y -= CamDirSpeed;
        }
        if (Input.IsKeyDown(EKeycode::Right))
        {
            Camera->Dir.Y += CamDirSpeed;
        }
        if (Input.IsKeyDown(EKeycode::Up))
        {
            Camera->Dir.X -= CamDirSpeed;
        }
        if (Input.IsKeyDown(EKeycode::Down))
        {
            Camera->Dir.X += CamDirSpeed;
        }

        VMatrix44 Rot = VMatrix44::Identity;
        f32 Speed = 0.1f * DeltaTime;
        if (Input.IsKeyDown(EKeycode::Q)) Rot.BuildRotationXYZ(0, Speed, 0);
        if (Input.IsKeyDown(EKeycode::E)) Rot.BuildRotationXYZ(0, -Speed, 0);
        if (Input.IsKeyDown(EKeycode::F)) Rot.BuildRotationXYZ(0, 0, Speed);
        if (Input.IsKeyDown(EKeycode::G)) Rot.BuildRotationXYZ(0, 0, -Speed);
        if (Input.IsKeyDown(EKeycode::Z)) Rot.BuildRotationXYZ(Speed, 0, 0);
        if (Input.IsKeyDown(EKeycode::X)) Rot.BuildRotationXYZ(-Speed, 0, 0);
        Object->Transform(Rot, ETransformType::LocalOnly, true);
    }
};

int main(int Argc, char** Argv)
{
    VWindowSpecification WindowSpec;
    VRenderSpecification RenderSpec;

    Volition.StartUp<GMyGameFlow>(WindowSpec, RenderSpec);
    Volition.Run();

    return 0;
}