/* TODO(sean):
    - Object -> RenderList
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
VCam4DV1 Cam;
VObject4DV1 Object;
VSurface Surface;
VRenderList4DV1 RenderList;

DEFINE_LOG_CHANNEL(hLogGame, "Game");

void VGame::StartUp()
{
    Object.LoadPLG(
        "tank3.plg",
        { 0.0f, 0.0f, 100.0f },
        { 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f }
    );
    RenderList.Reset();
    RenderList.InsertObject(Object);

    Cam.Init(0, { 0, 0, 0 }, { 0, 0, 0 }, Object.WorldPos, 100, 50, 500, { (f32)Renderer.GetScreenWidth(), (f32)Renderer.GetScreenHeight()});
}

void VGame::ShutDown()
{
}

void VGame::Update(f32 Delta)
{
    if (Input.IsKeyDown(EKeycode::Escape))
        Volition.Stop();

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
    // Camera
    {
        Cam.BuildWorldToCameraEulerMat44();
        // Cam.BuildWorldToCameraUVNMat44(EUVNMode::Simple);
    }

    // Object
    {
        Object.Reset();
        Object.TransModelToWorld();
        Object.Cull(Cam);
        Object.RemoveBackFaces(Cam);
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
                Object.RenderWire(Buffer, Pitch);
            /*
                Renderer.DrawTriangle(
                    Buffer, Pitch,
                    100, 100,
                    200, 200,
                    0, 200,
                    _RGB32(0xFF, 0xFF, 0xFF)
                );
            */
        }
        Renderer.BackSurface.Unlock();
    }

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
}
