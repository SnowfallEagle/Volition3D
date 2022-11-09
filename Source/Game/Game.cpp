#include "Core/Volition.h"
#include "Input/Input.h"
#include "Graphics/Graphics.h"
#include "Math/Math.h"
#include "Game/Game.h"
#include "Graphics/Renderer.h" // Of course in real game we don't need renderer's header but who cares

DEFINE_LOG_CHANNEL(hLogGame, "Game");

VGame Game;

// DEBUG(sean)
struct VGDebug
{
    VSurface* Surface;
    VSurface* Surface2;
    u32 Alpha;
    VObject4DV1 Object;
} static GDebug; // Game Debug

void VGame::StartUp()
{
    GDebug.Surface2 = VSurface::Load("Test2.bmp");
    GDebug.Surface = VSurface::Load("Test.bmp");
    GDebug.Alpha = 255u;
    GDebug.Object.LoadPLG(
        "Test.plg",
        { 0.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f },
        { 0, 0, 0 }
    );
}

void VGame::ShutDown()
{
    GDebug.Surface->Destroy();
    delete GDebug.Surface;

    GDebug.Surface2->Destroy();
    delete GDebug.Surface2;
}

void VGame::Update(f32 Delta)
{
    if (Input.IsKeyDown(EKeycode::Escape))
        Volition.Stop();



    /* DrawSurfaceBlended test
    {
        GDebug.Alpha %= 256u;
        if (Input.IsKeyDown(EKeycode::Left))
            GDebug.Alpha -= 1;
        if (Input.IsKeyDown(EKeycode::Right))
            GDebug.Alpha += 1;
        VL_LOG("%u\n", GDebug.Alpha);

        i32f Width = GDebug.Surface->GetWidth();
        i32f Height = GDebug.Surface->GetHeight();

        u32* Buffer;
        i32 Pitch;
        GDebug.Surface->Lock(Buffer, Pitch);
        {
            for (i32f Y = 0; Y < Height; ++Y)
            {
                for (i32f X = 0; X < Width; ++X)
                {
                    Buffer[X] = (Buffer[X] << 8) >> 8;
                    Buffer[X] |= GDebug.Alpha << _ALPHA_SHIFT;
                }
                Buffer += Pitch;
            }
        }
        GDebug.Surface->Unlock();
    }
    */
}

void VGame::Render()
{
    Graphics.DrawSurface(GDebug.Surface2, nullptr, nullptr);
    {
        VRelRectI Dest = { 0, 0, -128, -128 };
        Graphics.DrawSurfaceBlended(GDebug.Surface, nullptr, &Dest);
    }
    Graphics.DrawText(0, 0, _RGB32(0xFF, 0xFF, 0xFF), "FPS: %.3f", 1000.0f/Volition.GetDelta());
}
