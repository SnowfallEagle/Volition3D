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
    VSurface* Surface2;
    u32 Alpha;
} static GDebug; // Game Debug

void VGame::StartUp()
{
    GDebug.Surface2 = VSurface::Load("Test2.bmp");
    GDebug.Surface = VSurface::Load("Test.bmp");
    GDebug.Alpha = 255u;
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

    {
        float A = 1.0f;
        float B = 0.5f;
        __asm
        {
            fld     A
            fld     B
            fdiv
            fstp    A
        }

        if (Input.IsKeyDown(EKeycode::Return))
            VL_LOG("%f\n", A);
    }
}

void VGame::Render()
{
    Graphics.DrawSurface(GDebug.Surface2, nullptr, nullptr);
    {
        VRelRectI Dest = { 128, 128, -128, -128 };
        Graphics.DrawSurfaceBlended(GDebug.Surface, nullptr, &Dest);
    }
    Graphics.DrawText(0, 0, _RGB32(0xFF, 0xFF, 0xFF), "FPS: %.3f", 1000.0f/Volition.GetDelta());
}
