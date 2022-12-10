/* TODO:
    - Fix backfaces removal
    - Sometimes triangles don't rasterized fully, maybe floating point problem
 */

#include "Core/Volition.h"
#include "Input/Input.h"
#include "Math/Minimal.h"
#include "Graphics/Renderer.h"
#include "Graphics/Object.h"
#include "Graphics/RenderList.h"
#include "Game/Game.h"

VGame Game;

// DEBUG(sean): Variables to test things
static VCam4DV1 Cam;
static VObject4DV1 Object;
static VSurface Surface;
static VRenderList4DV1 RenderList;
static VVector2DI V1 = { 300, 90 };
static VVector2DI V2 = { -100, 180 };
static VVector2DI V3 = { 100, -20 };
static b32 bRenderSolid = true;
static b32 bBackFaceRemoval = true;
static u32 RenderKeyTicks = 0;
static u32 BackFaceKeyTicks = 0;

DEFINE_LOG_CHANNEL(hLogGame, "Game");

void VGame::StartUp()
{
    Object.LoadPLG(
        "tower1.plg",
        { 0.0f, 0.0f, 200.0f },
        { 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f }
    );

    Cam.Init(0, { 0, 0, 0 }, { 0, 0, 0 }, Object.WorldPos, 90, 50, 500, { (f32)Renderer.GetScreenWidth(), (f32)Renderer.GetScreenHeight()});
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

#if 0
    if (Input.IsKeyDown(EKeycode::W))
    {
        V1.Y -= 1;
        V2.Y -= 1;
        V3.Y -= 1;
    }
    if (Input.IsKeyDown(EKeycode::A))
    {
        V1.X -= 1;
        V2.X -= 1;
        V3.X -= 1;
    }
    if (Input.IsKeyDown(EKeycode::S))
    {
        V1.Y += 1;
        V2.Y += 1;
        V3.Y += 1;
    }
    if (Input.IsKeyDown(EKeycode::D))
    {
        V1.X += 1;
        V2.X += 1;
        V3.X += 1;
    }
#endif

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
        Cam.Pos.Z += 0.5f;
    }
    if (Input.IsKeyDown(EKeycode::S))
    {
        Cam.Pos.Z -= 0.5f;
    }
    if (Input.IsKeyDown(EKeycode::A))
    {
        Cam.Pos.X -= 0.5f;
    }
    if (Input.IsKeyDown(EKeycode::D))
    {
        Cam.Pos.X += 0.5f;
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
#if 0
    u32* Buffer;
    i32 Pitch;
    Renderer.BackSurface.Lock(Buffer, Pitch);
    {
        Renderer.DrawTriangle(Buffer, Pitch, V1.X,V1.Y, V2.X,V2.Y, V3.X,V3.Y, _RGB32(0xFF, 0xFF, 0xFF));
    }
    Renderer.BackSurface.Unlock();
#endif

    // Camera
    {
        Cam.BuildWorldToCameraEulerMat44();
        // Cam.BuildWorldToCameraUVNMat44(EUVNMode::Simple);
    }

#if 0
    // Object
    {
        Object.Reset();
        Object.TransModelToWorld();
        Object.Cull(Cam);
        if (bBackFaceRemoval)
        {
            Object.RemoveBackFaces(Cam);
        }
        Object.TransWorldToCamera(Cam.MatCamera);
        /*
        {
            Object.TransCameraToScreen(Cam);
        }
        */
        {
            Object.TransCameraToPerspective(Cam);
            Object.ConvertFromHomogeneous();
            Object.TransPerspectiveToScreen(Cam);
        }
    }

    // Render
    {
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
    }
#endif

    // RenderList
    {
        RenderList.Reset();
        RenderList.InsertObject(Object, true);
        RenderList.TransModelToWorld(Object.WorldPos);
        if (bBackFaceRemoval)
        {
            RenderList.RemoveBackFaces(Cam);
        }
        RenderList.TransWorldToCamera(Cam.MatCamera);
        {
            RenderList.TransCameraToScreen(Cam);
        }
        /*
        {
            RenderList.TransCameraToPerspective(Cam);
            RenderList.ConvertFromHomogeneous();
            RenderList.TransPerspectiveToScreen(Cam);
        }
        */
    }

    // Render
    {
        u32* Buffer;
        i32 Pitch;
        Renderer.BackSurface.Lock(Buffer, Pitch);
        {
            if (~Object.State & EObjectStateV1::Culled)
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
        }
        Renderer.BackSurface.Unlock();
    }

#if 0
    // Object info
    {
        Renderer.DrawText(0, 3, _RGB32(0xFF, 0xFF, 0xFF), "Name: %s", Object.Name);
        Renderer.DrawText(0, 33, _RGB32(0xFF, 0xFF, 0xFF), "Num vertices: %d", Object.NumVtx);
        Renderer.DrawText(0, 63, _RGB32(0xFF, 0xFF, 0xFF), "Num poly: %d", Object.NumPoly);
        Renderer.DrawText(0, 93, _RGB32(0xFF, 0xFF, 0xFF), "World pos: <%.2f, %.2f, %.2f>", Object.WorldPos.X, Object.WorldPos.Y, Object.WorldPos.Z);
        Renderer.DrawText(0, 123, _RGB32(0xFF, 0xFF, 0xFF), "State: 0x%x", Object.State);
        Renderer.DrawText(0, 153, _RGB32(0xFF, 0xFF, 0xFF), "Attr: 0x%x", Object.Attr);
        Renderer.DrawText(0, 183, _RGB32(0xFF, 0xFF, 0xFF), "Avg radius: %.3f", Object.AvgRadius);
        Renderer.DrawText(0, 213, _RGB32(0xFF, 0xFF, 0xFF), "Max radius: %.3f", Object.MaxRadius);
        Renderer.DrawText(0, 243, _RGB32(0xFF, 0xFF, 0xFF), "FPS: %.3f", 1000.0f / Volition.GetDelta());
    }
#else
    Renderer.DrawText(0, 5, _RGB32(0xCC, 0xCC, 0xCC), "FPS: %.3f", 1000.0f / Volition.GetDelta());
    Renderer.DrawText(0, 35, _RGB32(0xCC, 0xCC, 0xCC), bBackFaceRemoval ? "BackFace: true" : "BackFace: false");
    Renderer.DrawText(0, 65, _RGB32(0xCC, 0xCC, 0xCC), bRenderSolid ? "Render: Solid" : "Render: Wire");
#endif
}
