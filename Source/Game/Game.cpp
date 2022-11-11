#include "Core/Volition.h"
#include "Input/Input.h"
#include "Graphics/Graphics.h"
#include "Math/Minimal.h"
#include "Graphics/Renderer.h" // Of course in real game we don't need renderer's header but who cares
#include "Game/Game.h"

VGame Game;

// DEBUG(sean)
VSurface* Surface;
VSurface* Surface2;
u32 Alpha;
VObject4DV1 Object;

DEFINE_LOG_CHANNEL(hLogGame, "Game");

void VGame::StartUp()
{
    Surface2 = VSurface::Load("Test2.bmp");
    Surface = VSurface::Load("Test.bmp");
    Alpha = 255u;

    Object.LoadPLG(
        "Test.plg",
        { 0.0f, 0.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f, 0.0f }
    );

    VRenderList4DV1 List;
    List.NumPoly = 1;
    List.PolyList[0] = {
        0, 0, 0,
        { { 0, 0, 0, 1 }, { 1, 1, 1, 1 }, { 2, 2, 2, 1 } },
        { { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 } },
    };
    List.PolyPtrList[0] = &List.PolyList[0];

    VMatrix44 M = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        50, 25, -10, 1
    };

    VL_LOG("\n");
    for (i32f I = 0; I < 3; ++I)
    {
        List.PolyList[0].LocalVtx[I].Print();
        VL_LOG("\n");
    }

    List.Transform(M, ETransformType::TransOnly);

    VL_LOG("\n");
    for (i32f I = 0; I < 3; ++I)
    {
        List.PolyList[0].LocalVtx[I].Print();
        VL_LOG("\n");
    }

    VL_LOG("\n");
    for (i32f I = 0; I < 3; ++I)
    {
        List.PolyList[0].TransVtx[I].Print();
        VL_LOG("\n");
    }
}

void VGame::ShutDown()
{
    Surface->Destroy();
    delete Surface;

    Surface2->Destroy();
    delete Surface2;
}

void VGame::Update(f32 Delta)
{
    if (Input.IsKeyDown(EKeycode::Escape))
        Volition.Stop();

    /* DrawSurfaceBlended test
    {
        Alpha %= 256u;
        if (Input.IsKeyDown(EKeycode::Left))
            Alpha -= 1;
        if (Input.IsKeyDown(EKeycode::Right))
            Alpha += 1;
        VL_LOG("%u\n", GDebug.Alpha);

        i32f Width = Surface->GetWidth();
        i32f Height = Surface->GetHeight();

        u32* Buffer;
        i32 Pitch;
        Surface->Lock(Buffer, Pitch);
        {
            for (i32f Y = 0; Y < Height; ++Y)
            {
                for (i32f X = 0; X < Width; ++X)
                {
                    Buffer[X] = (Buffer[X] << 8) >> 8;
                    Buffer[X] |= Alpha << _ALPHA_SHIFT;
                }
                Buffer += Pitch;
            }
        }
        Surface->Unlock();
    }
    */
}

void VGame::Render()
{
    Graphics.DrawSurface(Surface2, nullptr, nullptr);
    {
        VRelRectI Dest = { 0, 0, -128, -128 };
        Graphics.DrawSurfaceBlended(Surface, nullptr, &Dest);
    }
    Graphics.DrawText(0, 0, _RGB32(0xFF, 0xFF, 0xFF), "FPS: %.3f", 1000.0f/Volition.GetDelta());
}
