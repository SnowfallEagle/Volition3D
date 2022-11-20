#include "Core/Volition.h"
#include "Input/Input.h"
#include "Graphics/Renderer.h"
#include "Math/Minimal.h"
#include "Graphics/Object.h"
#include "Game/Game.h"

VGame Game;

// DEBUG(sean): Variables to test things
VObject4DV1 Object;
VSurface Surface;

DEFINE_LOG_CHANNEL(hLogGame, "Game");

void VGame::StartUp()
{
    Object.LoadPLG(
        "tank3.plg",
        { 0.0f, 0.0f, 100.0f },
        { 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f }
    );

    VCam4DV1 Cam;
    Cam.Init(0, { 0, 0, 0 }, { -20.0f, 0.0f, 0.0f }, { 0, 0, 0 }, 90, 100, 1000, { (f32)Renderer.GetScreenWidth(), (f32)Renderer.GetScreenHeight()});
    Cam.BuildWorldToCameraEulerMat44();

    Object.TransModelToWorld();
    Object.Cull(Cam);
    Object.RemoveBackFaces(Cam);
    Object.TransWorldToCamera(Cam.MatCamera);
    Object.TransCameraToScreen(Cam);
    /*
    Object.TransCameraToPerspective(Cam);
    Object.ConvertFromHomogeneous();
    Object.TransPerspectiveToScreen(Cam);
    */
    Object.Reset();

    Surface.Load("test.bmp");
    u32* Buffer;
    i32 Pitch;
    Surface.Lock(Buffer, Pitch);
    for (i32f Y = 0; Y < Surface.GetHeight(); ++Y)
    {
        for (i32f X = 0; X < Surface.GetWidth(); ++X)
        {
            Buffer[X] |= 0xCC << _ALPHA_SHIFT;
        }
        Buffer += Pitch;
    }
    Surface.Unlock();
}

void VGame::ShutDown()
{
}

void VGame::Update(f32 Delta)
{
    if (Input.IsKeyDown(EKeycode::Escape))
        Volition.Stop();
}

void VGame::Render()
{
    u32* Buffer;
    i32 Pitch;
    Renderer.BackSurface.Lock(Buffer, Pitch);
    Object.RenderWire(Buffer, Pitch);
    Renderer.BackSurface.Unlock();

    Renderer.DrawText(0, 0, _RGB32(0xFF, 0xFF, 0xFF), "FPS: %.3f", 1000.0f/Volition.GetDelta());
}
