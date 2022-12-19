/* TODO:
    - Check camera's functions for matrices
 */

#include "Core/Volition.h"
#include "Input/Input.h"
#include "Math/Minimal.h"
#include "Graphics/Renderer.h"
#include "Graphics/Object.h"
#include "Graphics/RenderList.h"
#include "Game/Game.h"

VGame Game;

static VCam4DV1 Cam;
static VObject4DV1 Object;
static VSurface Surface;
static VRenderList4DV1 RenderList;

static b32 bRenderSolid = true;
static b32 bBackFaceRemoval = true;
static u32 RenderKeyTicks = 0;
static u32 BackFaceKeyTicks = 0;

DEFINE_LOG_CHANNEL(hLogGame, "Game");

void VGame::StartUp()
{
    Object.LoadPLG(
        "tank3.plg",
        { 0.0f, 0.0f, 50.0f },
        { 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f }
    );

    Cam.Init(ECamAttrV1::Euler, { 0, 0, 0 }, { 0, 0, 0 }, Object.WorldPos, 120, 265, 12000, { (f32)Renderer.GetScreenWidth(), (f32)Renderer.GetScreenHeight()});

    {
        VLightV1 AmbientLight = {
            0,
            ELightStateV1::Active,
            ELightAttrV1::Ambient,

            MAP_RGBX32(0xFF, 0xFF, 0xFF), 0, 0,
            { 0, 0, 0, 0 }, { 0, 0, 0, 0 },

            0, 0, 0,
            0, 0,
            0
        };

        VLightV1 InfiniteLight = {
            1,
            ELightStateV1::Active,
            ELightAttrV1::Infinite,

            0, MAP_RGBX32(0xFF, 0xFF, 0x00), 0,
            { 0, 0, 0, 0 }, { 0, -1.0f, 0, 0 },

            0, 0, 0,
            0, 0,
            0
        };

        VLightV1 PointLight = {
            2,
            ELightStateV1::Active,
            ELightAttrV1::Point,

            0, MAP_RGBX32(0xFF, 0xFF, 0x00), 0,
            { 0, 10000.0f, 0, 0 }, { 0, 0, 0, 0 },

            0, 1, 0,
            0, 0,
            0
        };

        VLightV1 Spotlight = {
            3,
            ELightStateV1::Active,
            ELightAttrV1::SimpleSpotlight,

            0, 0, MAP_RGBX32(0x80, 0x80, 0x80),
            { 1000.0f, 1000.0f, -1000.0f, 0 }, { -1.0f, -1.0f, 1.0f, 0 },

            0, 0, 0,
            30.0f, 60.0f,
            1.0f
        };

        Renderer.InitLight(0, AmbientLight);
        Renderer.InitLight(1, InfiniteLight);
        Renderer.InitLight(2, PointLight);
        Renderer.InitLight(3, Spotlight);
    }
}

void VGame::ShutDown()
{
}

void VGame::Update(f32 Delta)
{
    if (Input.IsKeyDown(EKeycode::Escape))
    {
        Volition.Stop();
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
        Cam.Pos.X += Math.FastSin(Cam.Dir.Y);
        Cam.Pos.Z += Math.FastCos(Cam.Dir.Y);
    }
    if (Input.IsKeyDown(EKeycode::S))
    {
        Cam.Pos.X -= Math.FastSin(Cam.Dir.Y);
        Cam.Pos.Z -= Math.FastCos(Cam.Dir.Y);
    }

    if (Input.IsKeyDown(EKeycode::Left))
    {
        Cam.Dir.Y -= 0.5f;
    }
    if (Input.IsKeyDown(EKeycode::Right))
    {
        Cam.Dir.Y += 0.5f;
    }
    if (Input.IsKeyDown(EKeycode::Up))
    {
        Cam.Dir.X -= 0.5f;
    }
    if (Input.IsKeyDown(EKeycode::Down))
    {
        Cam.Dir.X += 0.5f;
    }

    if (Input.IsKeyDown(EKeycode::Q))
    {
        VMatrix44 Rot;
        Rot.BuildRotationXYZ(0.0f, 0.5f, 0);
        Object.Transform(Rot, ETransformType::LocalOnly, true);
    }

    if (Input.IsKeyDown(EKeycode::E))
    {
        VMatrix44 Rot;
        Rot.BuildRotationXYZ(0.0f, -0.5f, 0.0f);
        Object.Transform(Rot, ETransformType::LocalOnly, true);
    }
}

void VGame::Render()
{
    Cam.BuildWorldToCameraEulerMat44();

    RenderList.Reset();
    Object.Reset();

    Object.TransformModelToWorld();
    Object.Cull(Cam);

#if 0
    if (bBackFaceRemoval)
    {
        Object.RemoveBackFaces(Cam);
    }
    Object.TransformWorldToCamera(Cam.MatCamera);
    {
        Object.TransformCameraToScreen(Cam);
    }
    {
        // Object.TransformCameraToPerspective(Cam);
        // Object.TransformPerspectiveToScreen(Cam);
    }

    VRelRectI Dest = { 0, 0, Volition.WindowWidth, Volition.WindowHeight/2 };
    Renderer.BackSurface.FillRectHW(&Dest, MAP_XRGB32(100, 20, 255));
    Dest = { 0, Dest.H, Dest.W, Volition.WindowHeight / 2 - 1 };
    Renderer.BackSurface.FillRectHW(&Dest, MAP_XRGB32(60, 10, 255));

    u32* Buffer;
    i32 Pitch;
    Renderer.BackSurface.Lock(Buffer, Pitch);
    {
        if (~Object.State & EObjectStateV1::Culled)
        {
            if (bRenderSolid)
            {
                Object.RenderSolid(Buffer, Pitch);
            }
            else
            {
                Object.RenderWire(Buffer, Pitch);
            }
        }
    }
    Renderer.BackSurface.Unlock();

#else

    RenderList.InsertObject(Object, false);
    if (bBackFaceRemoval)
    {
        RenderList.RemoveBackFaces(Cam);
    }
    RenderList.TransformWorldToCamera(Cam.MatCamera);
    {
        RenderList.TransformCameraToScreen(Cam);
    }
    {
        // RenderList.TransformCameraToPerspective(Cam);
        // RenderList.TransformPerspectiveToScreen(Cam);
    }

    VRelRectI Dest = { 0, 0, Volition.WindowWidth, Volition.WindowHeight/2 };
    Renderer.BackSurface.FillRectHW(&Dest, MAP_XRGB32(100, 20, 255));
    Dest = { 0, Dest.H, Dest.W, Volition.WindowHeight / 2 - 1 };
    Renderer.BackSurface.FillRectHW(&Dest, MAP_XRGB32(60, 10, 255));

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
#endif

    Renderer.DrawText(0, 5, MAP_XRGB32(0xFF, 0xFF, 0xFF), "FPS: %.3f", 1000.0f / Volition.GetDelta());
    Renderer.DrawText(0, 35, MAP_XRGB32(0xFF, 0xFF, 0xFF), bBackFaceRemoval ? "BackFace: true" : "BackFace: false");
    Renderer.DrawText(0, 65, MAP_XRGB32(0xFF, 0xFF, 0xFF), bRenderSolid ? "Render: Solid" : "Render: Wire");
}
