#include "Engine/Core/Volition.h"
#include "Engine/Input/Input.h"
#include "Engine/Math/Minimal.h"
#include "Engine/Graphics/RenderContext.h"
#include "Engine/Graphics/Object.h"
#include "Engine/GameFramework/Game.h" // TODO(sean): It's stupid that game is in the GameFramework folder...

VGame Game;

static VCamera Camera;
static VObject* Object;
static VObject Objects[3];
static VSurface Surface;
static VRenderList RenderList;

static b32 bRenderSolid = true;
static b32 bBackFaceRemoval = true;
static u32 RenderKeyTicks = 0;
static u32 BackFaceKeyTicks = 0;

static VVector4 PositionVtx0 = { 600, 20, 1.0f };
static VVector4 PositionVtx1 = { 100, 20, 1.0f };
static VVector4 PositionVtx2 = { 1100, 650, 1.0f };

VL_DEFINE_LOG_CHANNEL(hLogGame, "Game");

void VGame::StartUp()
{
    Objects[0].LoadCOB(
        "rec_gouraud_textured_02.cob",
        { 0.0f, 0.0f, 250.0f },
        { 100.0f, 100.0f, 100.0f },
        { 0.0f, 0.0f, 0.0f }
    );
    Objects[1].LoadCOB(
        "s_alpha.cob",
        { 0.0f, 0.0f, 250.0f },
        { 100.0f, 100.0f, 100.0f },
        { 0.0f, 0.0f, 0.0f },
        ECOB::SwapYZ
    );
    Objects[2].LoadCOB(
        "jetski05.cob",
        { 0.0f, 0.0f, 250.0f },
        { 100.0f, 100.0f, 100.0f },
        { 0.0f, 0.0f, 0.0f },
        ECOB::SwapYZ | ECOB::InvertV
    );
    Object = &Objects[0];

    Camera.Init(ECameraAttr::Euler, { 0, 75.0f, 0 }, { 0, 0, 0 }, Object->Position, 90, 50, 12000, { (f32)Renderer.GetScreenWidth(), (f32)Renderer.GetScreenHeight()});
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

void VGame::ShutDown()
{
    Objects[0].Destroy();
    Objects[1].Destroy();
    Objects[2].Destroy();
}

void VGame::Update(f32 DeltaTime)
{
    if (Input.IsKeyDown(EKeycode::Escape))
    {
        Volition.Stop();
    }

    if (Input.IsKeyDown(EKeycode::N1)) Object = &Objects[0];
    if (Input.IsKeyDown(EKeycode::N2)) Object = &Objects[1];
    if (Input.IsKeyDown(EKeycode::N3)) Object = &Objects[2];

    f32 SpeedTri = 0.2f * DeltaTime;
    if (Input.IsKeyDown(EKeycode::Up))
    {
        PositionVtx0.Y -= SpeedTri;
        PositionVtx1.Y -= SpeedTri;
        PositionVtx2.Y -= SpeedTri;
    }
    if (Input.IsKeyDown(EKeycode::Right))
    {
        PositionVtx0.X += SpeedTri;
        PositionVtx1.X += SpeedTri;
        PositionVtx2.X += SpeedTri;
    }
    if (Input.IsKeyDown(EKeycode::Left))
    {
        PositionVtx0.X -= SpeedTri;
        PositionVtx1.X -= SpeedTri;
        PositionVtx2.X -= SpeedTri;
    }
    if (Input.IsKeyDown(EKeycode::Down))
    {
        PositionVtx0.Y += SpeedTri;
        PositionVtx1.Y += SpeedTri;
        PositionVtx2.Y += SpeedTri;
    }

    if (Input.IsKeyDown(EKeycode::R) && Time.GetTicks() - RenderKeyTicks > 100)
    {
        bRenderSolid = !bRenderSolid;
        RenderKeyTicks = Time.GetTicks();
    }
    if (Input.IsKeyDown(EKeycode::B) && Time.GetTicks() - BackFaceKeyTicks > 100)
    {
        bBackFaceRemoval = !bBackFaceRemoval;
        BackFaceKeyTicks = Time.GetTicks();
    }

    f32 CamPosSpeed = 0.1f * DeltaTime;
    if (Input.IsKeyDown(EKeycode::W))
    {
        Camera.Pos.X += Math.FastSin(Camera.Dir.Y) * CamPosSpeed;
        Camera.Pos.Z += Math.FastCos(Camera.Dir.Y) * CamPosSpeed;
    }
    if (Input.IsKeyDown(EKeycode::S))
    {
        Camera.Pos.X -= Math.FastSin(Camera.Dir.Y) * CamPosSpeed;
        Camera.Pos.Z -= Math.FastCos(Camera.Dir.Y) * CamPosSpeed;
    }

    f32 CamDirSpeed = 0.2f * DeltaTime;
    if (Input.IsKeyDown(EKeycode::Left))
    {
        Camera.Dir.Y -= CamDirSpeed;
    }
    if (Input.IsKeyDown(EKeycode::Right))
    {
        Camera.Dir.Y += CamDirSpeed;
    }
    if (Input.IsKeyDown(EKeycode::Up))
    {
        Camera.Dir.X -= CamDirSpeed;
    }
    if (Input.IsKeyDown(EKeycode::Down))
    {
        Camera.Dir.X += CamDirSpeed;
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

void VGame::Render()
{
    Camera.BuildWorldToCameraEulerMat44();

    // Proccess object
    {
        Object->Reset();
        Object->TransformModelToWorld();
        Object->Cull(Camera);
    }

    // Proccess render list
    {
        RenderList.Reset();
        RenderList.InsertObject(*Object, false);
        if (bBackFaceRemoval)
        {
            RenderList.RemoveBackFaces(Camera);
        }
        RenderList.TransformWorldToCamera(Camera);
        RenderList.Clip(Camera);
        Renderer.TransformLights(Camera);
        RenderList.Light(Camera, Renderer.Lights, Renderer.MaxLights);
        RenderList.SortPolygons(ESortPolygonsMethod::Average);
        RenderList.TransformCameraToScreen(Camera);
    }

    // Draw background
    {
        VRelRectI Dest = { 0, 0, Renderer.GetScreenWidth(), Renderer.GetScreenHeight()};
        Renderer.BackSurface.FillRectHW(&Dest, MAP_XRGB32(0x66, 0x00, 0x00));
        /*
        VRelRectI Dest = { 0, 0, Volition.WindowWidth, Volition.WindowHeight/2 };
        Renderer.BackSurface.FillRectHW(&Dest, MAP_XRGB32(100, 20, 255));
        Dest = { 0, Dest.H, Dest.W, Volition.WindowHeight / 2 - 1 };
        Renderer.BackSurface.FillRectHW(&Dest, MAP_XRGB32(60, 10, 255));
        */
    }

    // Render stuff
    u32* Buffer;
    i32 Pitch;
    Renderer.BackSurface.Lock(Buffer, Pitch);
    {
        if (bRenderSolid)
        {
            RenderList.RenderSolid(Buffer, Pitch);
        }
        else
        {
            RenderList.RenderWire(Buffer, Pitch);
        }
    }
    Renderer.BackSurface.Unlock();

    // Some debug info
    {
        Renderer.DrawText(0, 5, MAP_XRGB32(0xFF, 0xFF, 0xFF), "FPS: %.3f", 1000.0f / Time.GetDeltaTime());
        Renderer.DrawText(0, 35, MAP_XRGB32(0xFF, 0xFF, 0xFF), bBackFaceRemoval ? "Backface culling: true" : "Backface culling: false");
        Renderer.DrawText(0, 65, MAP_XRGB32(0xFF, 0xFF, 0xFF), bRenderSolid ? "Render mode: Solid" : "Render mode: Wire");
    }
}
