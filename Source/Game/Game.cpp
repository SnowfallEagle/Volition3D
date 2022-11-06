#include "Core/Volition.h"
#include "Input/Input.h"
#include "Graphics/Graphics.h"
#include "Math/Math.h"
#include "Game/Game.h"

DEFINE_LOG_CHANNEL(hLogGame, "Game");

VGame Game;

// DEBUG(sean)
struct VGDebug
{
    VSurface* Surface;
    fx16 Fx;
};
static VGDebug GDebug;

void VGame::StartUp()
{
    GDebug.Surface = VSurface::Load("Test.bmp");
    GDebug.Surface->EnableColorKey();
    GDebug.Fx = FloatToFx16(10.5f);
}

void VGame::ShutDown()
{
    GDebug.Surface->Destroy();
    delete GDebug.Surface;
}

void VGame::Update(f32 Delta)
{
    if (Input.IsKeyDown(EKeycode::Escape))
        Volition.Stop();

    fx16 Higher = FloatToFx16(1.01f);
    fx16 Lower = FloatToFx16(0.99f);

    if (Input.IsKeyDown(EKeycode::Up))
        GDebug.Fx = DivFx16(GDebug.Fx, Lower);
    if (Input.IsKeyDown(EKeycode::Down))
        GDebug.Fx = DivFx16(GDebug.Fx, Higher);
    if (Input.IsKeyDown(EKeycode::Left))
        VL_LOG("%d\n", Random(100));
    else if (Input.IsKeyDown(EKeycode::Right))
        VL_LOG("%d\n", Random(-100, 100));
}

void VGame::Render()
{
    Graphics.DrawSurface(GDebug.Surface, nullptr, nullptr);
    Graphics.DrawText(0, 0, _RGB32(0xFF, 0xFF, 0xFF), "FPS: %.3f", 1000.0f/Volition.GetDelta());
}
