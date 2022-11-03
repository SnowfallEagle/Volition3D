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

    VL_LOG("%.3f\n", Delta);
}

void VGame::Render()
{
    Graphics.DrawSurface(GDebug.Surface, nullptr, nullptr);
}
