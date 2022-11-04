#include "Core/Volition.h"
#include "Input/Input.h"
#include "Game/Game.h"

DEFINE_LOG_CHANNEL(hLogGame, "Game");

VGame Game;

// DEBUG(sean)
struct VGDebug
{
    VSurface* Surface;
};
static VGDebug GDebug;

void VGame::StartUp()
{
    GDebug.Surface = VSurface::Load("Test.bmp");
    GDebug.Surface->EnableColorKey();
}

void VGame::ShutDown()
{
    GDebug.Surface->Destroy();
    delete GDebug.Surface;
}

void VGame::Update(f32 Delta)
{
    if (Input.IsKeyDown(EKeyCode::Escape))
        Volition.Stop();
}

void VGame::Render()
{
    Graphics.DrawSurface(GDebug.Surface, nullptr, nullptr);
    Graphics.DrawText(0, 0, _RGB32(0xFF, 0xFF, 0xFF), "FPS: %.3f", 1000.0f/Volition.GetDelta());
}
