/* TODO:
 */

#include "Core/Volition.h"
#include "Input/Input.h"
#include "Math/Minimal.h"
#include "Graphics/Renderer.h"
#include "Graphics/Object.h"
#include "Graphics/RenderList.h"
#include "GameFramework/Game.h"

VGame Game;

static VCamera Camera;
static VObject Object;
static VSurface Surface;
static VRenderList RenderList;

static b32 bRenderSolid = true;
static b32 bBackFaceRemoval = true;
static u32 RenderKeyTicks = 0;
static u32 BackFaceKeyTicks = 0;

static VVector4 PositionVtx0 = { 600, 20, 1.0f };
static VVector4 PositionVtx1 = { 100, 20, 1.0f };
static VVector4 PositionVtx2 = { 1100, 650, 1.0f };

DEFINE_LOG_CHANNEL(hLogGame, "Game");

void VGame::StartUp()
{
#if 1
    Object.LoadCOB(
        "cube_flat_textured_01.cob",
        { 0.0f, 0.0f, 250.0f },
        { 30.0f, 30.0f, 30.0f },
        { 0.0f, 0.0f, 0.0f }
    );
#else
    Object.LoadPLG(
        "tankg3.plg",
        { 0.0f, 0.0f, 250.0f },
        { 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f }
    );
#endif

    Camera.Init(ECameraAttr::Euler, { 0, 75.0f, 0 }, { 0, 0, 0 }, Object.Position, 120, 265, 12000, { (f32)Renderer.GetScreenWidth(), (f32)Renderer.GetScreenHeight()});

    {
        VLight AmbientLight = {
            0,
            ELightState::Active,
            ELightAttr::Ambient,

            MAP_RGBX32(0x22, 0xFF, 0xFF), 0, 0,
            { 0, 0, 0, 0 }, VVector4{ 0, 0, 0, 0 }.GetNormalized(),

            0, 0, 0,
            0, 0,
            0
        };

        VLight InfiniteLight = {
            1,
            ELightState::Active,
            ELightAttr::Infinite,

            0, MAP_RGBX32(0xCC, 0xCC, 0xAA), 0,
            { 0, 0, 0, 0 }, VVector4{ -1.0f, -1.0f, 0, 0 }.GetNormalized(),

            0, 0, 0,
            0, 0,
            0
        };

        VLight PointLight = {
            2,
            ELightState::Active,
            ELightAttr::Point,

            0, MAP_RGBX32(0xFF, 0xFF, 0xff), 0,
            { 1000.0f, 1000.0f, 0, 0 }, VVector4{ 0, 0, 0, 0 }.GetNormalized(),

            0, 0.0001f, 0,
            0, 0,
            0
        };

        VLight SimpleSpotlight = {
            3,
            ELightState::Active,
            ELightAttr::SimpleSpotlight,

            0, MAP_RGBX32(0xAA, 0xAA, 0xAA), 0,
            { 1000.0f, 1000.0f, 0.0f, 0 }, VVector4(-1.0f, -1.0f, 0.0f).GetNormalized(),

            0, 0.0005f, 0,
            30.0f, 60.0f,
            1.0f
        };

        VLight ComplexSpotlight = {
            3,
            ELightState::Active,
            ELightAttr::ComplexSpotlight,

            0, MAP_RGBX32(0xFF, 0xFF, 0xFF), 0,
            { 0.0f, 1000.0f, -300.0f, 0 }, VVector4(-0.5f, -1.0f, 1.0f).GetNormalized(),

            0, 0.0005f, 0,
            30.0f, 60.0f,
            1.0f
        };

        //Renderer.InitLight(0, AmbientLight);
        Renderer.InitLight(0, ComplexSpotlight);
        //Renderer.InitLight(2, InfiniteLight);
        //Renderer.InitLight(3, PointLight);
        //Renderer.InitLight(0, SimpleSpotlight);
    }
}

void VGame::ShutDown()
{
    Object.Destroy();
}

void VGame::Update(f32 Delta)
{
    if (Input.IsKeyDown(EKeycode::Escape))
    {
        Volition.Stop();
    }

    f32 SpeedTri = 0.5f * Delta;
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

    if (Input.IsKeyDown(EKeycode::R) && Volition.GetTicks() - RenderKeyTicks > 100)
    {
        bRenderSolid = !bRenderSolid;
        RenderKeyTicks = Volition.GetTicks();
    }
    if (Input.IsKeyDown(EKeycode::B) && Volition.GetTicks() - BackFaceKeyTicks > 100)
    {
        bBackFaceRemoval = !bBackFaceRemoval;
        BackFaceKeyTicks = Volition.GetTicks();
    }

    if (Input.IsKeyDown(EKeycode::W))
    {
        Camera.Pos.X += Math.FastSin(Camera.Dir.Y);
        Camera.Pos.Z += Math.FastCos(Camera.Dir.Y);
    }
    if (Input.IsKeyDown(EKeycode::S))
    {
        Camera.Pos.X -= Math.FastSin(Camera.Dir.Y);
        Camera.Pos.Z -= Math.FastCos(Camera.Dir.Y);
    }

    if (Input.IsKeyDown(EKeycode::Left))
    {
        Camera.Dir.Y -= 0.5f;
    }
    if (Input.IsKeyDown(EKeycode::Right))
    {
        Camera.Dir.Y += 0.5f;
    }
    if (Input.IsKeyDown(EKeycode::Up))
    {
        Camera.Dir.X -= 0.5f;
    }
    if (Input.IsKeyDown(EKeycode::Down))
    {
        Camera.Dir.X += 0.5f;
    }

    VMatrix44 Rot = VMatrix44::Identity;
    f32 Speed = 0.1f * Delta;
    if (Input.IsKeyDown(EKeycode::Q)) Rot.BuildRotationXYZ(0, Speed, 0);
    if (Input.IsKeyDown(EKeycode::E)) Rot.BuildRotationXYZ(0, -Speed, 0);
    if (Input.IsKeyDown(EKeycode::F)) Rot.BuildRotationXYZ(0, 0, Speed);
    if (Input.IsKeyDown(EKeycode::G)) Rot.BuildRotationXYZ(0, 0, -Speed);
    if (Input.IsKeyDown(EKeycode::Z)) Rot.BuildRotationXYZ(Speed, 0, 0);
    if (Input.IsKeyDown(EKeycode::X)) Rot.BuildRotationXYZ(-Speed, 0, 0);
    Object.Transform(Rot, ETransformType::LocalOnly, true);
}

void VGame::Render()
{
    // Prepare camera
    Camera.BuildWorldToCameraEulerMat44();

    // Proccess object
    {
        Object.Reset();
        Object.TransformModelToWorld();
        Object.Cull(Camera);
    }

    // Proccess render list
    {
        RenderList.Reset();
        RenderList.InsertObject(Object, false);
        if (bBackFaceRemoval)
        {
            RenderList.RemoveBackFaces(Camera);
        }
        RenderList.Light(Camera, Renderer.Lights, Renderer.MaxLights);
        RenderList.TransformWorldToCamera(Camera);
        RenderList.SortPolygons(ESortPolygonsMethod::Average);
        RenderList.TransformCameraToScreen(Camera);
    }

    // Draw background
    {
        VRelRectI Dest = { 0, 0, Volition.WindowWidth, Volition.WindowHeight/2 };
        Renderer.BackSurface.FillRectHW(&Dest, MAP_XRGB32(100, 20, 255));
        Dest = { 0, Dest.H, Dest.W, Volition.WindowHeight / 2 - 1 };
        Renderer.BackSurface.FillRectHW(&Dest, MAP_XRGB32(60, 10, 255));
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
        Renderer.DrawText(0, 5, MAP_XRGB32(0xFF, 0xFF, 0xFF), "FPS: %.3f", 1000.0f / Volition.GetDelta());
        Renderer.DrawText(0, 35, MAP_XRGB32(0xFF, 0xFF, 0xFF), bBackFaceRemoval ? "Backface culling: true" : "Backface culling: false");
        Renderer.DrawText(0, 65, MAP_XRGB32(0xFF, 0xFF, 0xFF), bRenderSolid ? "Render mode: Solid" : "Render mode: Wire");
    }
}
