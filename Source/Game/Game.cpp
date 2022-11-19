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
        "Test.plg",
        { 0.0f, 0.0f, 800.0f },
        { 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f }
    );

    VCam4DV1 Cam;
    Cam.Init(0, { 0, 0, 0 }, { 0.0f, 0.0f, 0.0f }, { 0, 0, 0 }, 90, 100, 1000, { 1280, 720 });
    Cam.BuildWorldToCameraEulerMat44();

    Object.TransModelToWorld();
    Object.Cull(Cam);
    Object.RemoveBackFaces(Cam.Pos);
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
    Surface.DrawBlended(nullptr, &Renderer.BackSurface, nullptr);
    Renderer.DrawText(0, 0, _RGB32(0xFF, 0xFF, 0xFF), "FPS: %.3f", 1000.0f/Volition.GetDelta());
}
