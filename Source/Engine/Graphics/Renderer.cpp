#include "Engine/Core/Memory.h"
#include "Engine/Core/Window.h"
#include "Engine/Core/World.h"
#include "Engine/Graphics/Renderer.h"
#include "Engine/Graphics/RenderList.h"

VRenderer Renderer;

VL_DEFINE_LOG_CHANNEL(hLogRenderer, "Renderer");

void VRenderer::StartUp(const VRenderSpecification& InRenderSpec)
{
    // Copy render specification
    {
        RenderSpec = InRenderSpec;
    }

    // Get window surface and init pixel format
    SDL_Surface* SDLSurface;
    {
        SDLSurface = SDL_GetWindowSurface(Window.SDLWindow);
        VL_ASSERT(SDLSurface);

        RenderSpec.SDLPixelFormat = SDLSurface->format;
        RenderSpec.SDLPixelFormatEnum = RenderSpec.SDLPixelFormat->format;
    }

    // Create video and back surfaces
    {
        VideoSurface.Create(SDLSurface);

        RenderSpec.TargetSize = { VideoSurface.Width, VideoSurface.Height };

        RenderSpec.MinClip = { 0, 0 };
        RenderSpec.MaxClip = { RenderSpec.TargetSize.X - 1, RenderSpec.TargetSize.Y - 1 };

        RenderSpec.MinClipFloat = { (f32)RenderSpec.MinClip.X, (f32)RenderSpec.MinClip.Y };
        RenderSpec.MaxClipFloat = { (f32)RenderSpec.MaxClip.X, (f32)RenderSpec.MaxClip.Y };

        BackSurface.Create(RenderSpec.TargetSize.X, RenderSpec.TargetSize.Y);
    }

    // Init render context
    {
        RenderContext.Init();
    }

    // Initialize TTF
    {
        static constexpr i32f CharsPerLine = 80;
        static constexpr f32 PointDivPixel = 0.75f;
        static constexpr f32 QualityMultiplier = 2.0f;

        i32 Res = TTF_Init();
        VL_ASSERT(Res == 0);

        FontCharWidth = RenderSpec.TargetSize.X / CharsPerLine;
        FontCharHeight = (i32)(FontCharWidth * 1.25f);

        Font = TTF_OpenFont("Default.ttf", (i32)( (f32)FontCharWidth * PointDivPixel * QualityMultiplier ));
        VL_ASSERT(Font);
    }

    // Log
    VL_NOTE(hLogRenderer, "Initialized with %s pixel format\n", SDL_GetPixelFormatName(RenderSpec.SDLPixelFormatEnum));
}

void VRenderer::ShutDown()
{
    // Shut down TTF
    {
        TTF_CloseFont(Font);
        TTF_Quit();
    }

    // Free renderer stuff
    {
        RenderContext.Destroy();
        BackSurface.Destroy();
    }
}

void VRenderer::Flip()
{
    SDL_BlitSurface(BackSurface.SDLSurface, nullptr, VideoSurface.SDLSurface, nullptr);
    SDL_UpdateWindowSurface(Window.SDLWindow);
}

void VRenderer::DrawLine(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, u32 Color)
{
    i32f DX, DY, DX2, DY2, XInc, YInc, Error;

    // Compute deltas
    DX = X2 - X1;
    DY = Y2 - Y1;

    // Get X/Y increments and positive deltas
    if (DX >= 0)
    {
        XInc = 1;
    }
    else
    {
        XInc = -1;
        DX = -DX;
    }

    if (DY >= 0)
    {
        YInc = Pitch;
    }
    else
    {
        YInc = -Pitch;
        DY = -DY;
    }

    DX2 = DX << 1;
    DY2 = DY << 1;

    // Compute buffer start
    Buffer += Y1 * Pitch + X1;

    if (DX > DY)
    {
        Error = DY2 - DX;

        for (i32f I = 0; I < DX; ++I)
        {
            *Buffer = Color;

            if (Error >= 0)
            {
                Error -= DX2;
                Buffer += YInc;
            }

            Error += DY2;
            Buffer += XInc;
        }
    }
    else
    {
        Error = DX2 - DY;

        for (i32f I = 0; I < DY; ++I)
        {
            *Buffer = Color;

            if (Error >= 0)
            {
                Error -= DY2;
                Buffer += XInc;
            }

            Error += DX2;
            Buffer += YInc;
        }
    }
}

void VRenderer::DrawLineSlow(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, u32 Color)
{
    if (Math.Abs(X2 - X1) > Math.Abs(Y2 - Y1))
    {
        i32 T;
        if (X1 > X2)
        {
            VL_SWAP(X1, X2, T);
            VL_SWAP(Y1, Y2, T);
        }

        const f32 M = (f32)(Y2 - Y1) / (f32)(X2 - X1);
        const f32 B = Y1 - X1 * M;

        for (i32 X = X1; X < X2; ++X)
        {
            f32 Y = M * (f32)X + B;
            Buffer[(i32f)Y * Pitch + X] = Color;
        }
    }
    else
    {
        i32 T;
        if (Y1 > Y2)
        {
            VL_SWAP(X1, X2, T);
            VL_SWAP(Y1, Y2, T);
        }

        const f32 M = (f32)(X2 - X1) / (f32)(Y2 - Y1);
        const f32 B = X1 - Y1 * M;

        for (i32 Y = Y1; Y < Y2; ++Y)
        {
            f32 X = M * (f32)Y + B;
            Buffer[Y * Pitch + (i32f)X] = Color;
        }
    }
}

b32 VRenderer::ClipLine(i32& X1, i32& Y1, i32& X2, i32& Y2) const
{
    /* NOTE(sean):
        We can optimize this function if we throw away floating
        point computations, but we could loss in accuracy...
     */

    enum EClipCode
    {
        CC_N = VL_BIT(1),
        CC_W = VL_BIT(2),
        CC_S = VL_BIT(3),
        CC_E = VL_BIT(4),

        CC_NW = CC_N | CC_W,
        CC_NE = CC_N | CC_E,
        CC_SW = CC_S | CC_W,
        CC_SE = CC_S | CC_E,
    };

    i32 CX1 = X1, CY1 = Y1, CX2 = X2, CY2 = Y2;
    i32 Code1 = 0, Code2 = 0;

    // Define codes
    if (CX1 < RenderSpec.MinClip.X)
        Code1 |= CC_W;
    else if (CX1 > RenderSpec.MaxClip.X)
        Code1 |= CC_E;
    if (CY1 < RenderSpec.MinClip.Y)
        Code1 |= CC_N;
    else if (CY1 > RenderSpec.MaxClip.Y)
        Code1 |= CC_S;

    if (CX2 < RenderSpec.MinClip.X)
        Code2 |= CC_W;
    else if (CX2 > RenderSpec.MaxClip.X)
        Code2 |= CC_E;
    if (CY2 < RenderSpec.MinClip.Y)
        Code2 |= CC_N;
    else if (CY2 > RenderSpec.MaxClip.Y)
        Code2 |= CC_S;

    // Center is 0, so check if we can't see whole line
    if (Code1 & Code2)
        return false;

    // We see whole line, don't need to clip
    if (Code1 == 0 && Code2 == 0)
        return true;

    // Clip line
    switch (Code1)
    {
    case CC_N:
    {
        CY1 = RenderSpec.MinClip.Y;
        CX1 = (i32)(
            0.5f + (X1 + (CY1 - Y1) * (X2-X1) / (f32)(Y2-Y1))
        );
    } break;

    case CC_S:
    {
        CY1 = RenderSpec.MaxClip.Y;
        CX1 = (i32)(
            0.5f + (X1 + (CY1 - Y1) * (X2-X1) / (f32)(Y2-Y1))
        );
    } break;

    case CC_W:
    {
        CX1 = RenderSpec.MinClip.X;
        CY1 = (i32)(
            0.5f + (Y1 + (CX1 - X1) * (Y2-Y1) / (f32)(X2-X1))
        );
    } break;

    case CC_E:
    {
        CX1 = RenderSpec.MaxClip.X;
        CY1 = (i32)(
            0.5f + (Y1 + (CX1 - X1) * (Y2-Y1) / (f32)(X2-X1))
        );
    } break;

    case CC_NW:
    {
        CY1 = RenderSpec.MinClip.Y;
        CX1 = (i32)(
            0.5f + (X1 + (CY1 - Y1) * (X2-X1) / (f32)(Y2-Y1))
        );

        if (CX1 < RenderSpec.MinClip.X || CX1 > RenderSpec.MaxClip.X)
        {
            CX1 = RenderSpec.MinClip.X;
            CY1 = (i32)(
                0.5f + (Y1 + (CX1 - X1) * (Y2-Y1) / (f32)(X2-X1))
            );
        }
    } break;

    case CC_NE:
    {
        CY1 = RenderSpec.MinClip.Y;
        CX1 = (i32)(
            0.5f + (X1 + (CY1 - Y1) * (X2-X1) / (f32)(Y2-Y1))
        );

        if (CX1 < RenderSpec.MinClip.X || CX1 > RenderSpec.MaxClip.X)
        {
            CX1 = RenderSpec.MaxClip.X;
            CY1 = (i32)(
                0.5f + (Y1 + (CX1 - X1) * (Y2-Y1) / (f32)(X2-X1))
            );
        }
    } break;

    case CC_SW:
    {
        CY1 = RenderSpec.MaxClip.Y;
        CX1 = (i32)(
            0.5f + (X1 + (CY1 - Y1) * (X2-X1) / (f32)(Y2-Y1))
        );

        if (CX1 < RenderSpec.MinClip.X || CX1 > RenderSpec.MaxClip.X)
        {
            CX1 = RenderSpec.MinClip.X;
            CY1 = (i32)(
                0.5f + (Y1 + (CX1 - X1) * (Y2-Y1) / (f32)(X2-X1))
            );
        }
    } break;

    case CC_SE:
    {
        CY1 = RenderSpec.MaxClip.Y;
        CX1 = (i32)(
            0.5f + (X1 + (CY1 - Y1) * (X2-X1) / (f32)(Y2-Y1))
        );

        if (CX1 < RenderSpec.MinClip.X || CX1 > RenderSpec.MaxClip.X)
        {
            CX1 = RenderSpec.MaxClip.X;
            CY1 = (i32)(
                0.5f + (Y1 + (CX1 - X1) * (Y2-Y1) / (f32)(X2-X1))
            );
        }
    } break;

    default: {} break;
    }

    switch (Code2)
    {
    case CC_N:
    {
        CY2 = RenderSpec.MinClip.Y;
        CX2 = (i32)(
            0.5f + (X2 + (CY2 - Y2) * (X1-X2) / (f32)(Y1-Y2))
        );
    } break;

    case CC_S:
    {
        CY2 = RenderSpec.MaxClip.Y;
        CX2 = (i32)(
            0.5f + (X2 + (CY2 - Y2) * (X1-X2) / (f32)(Y1-Y2))
        );
    } break;

    case CC_W:
    {
        CX2 = RenderSpec.MinClip.X;
        CY2 = (i32)(
            0.5f + (Y2 + (CX2 - X2) * (Y1-Y2) / (f32)(X1-X2))
        );
    } break;

    case CC_E:
    {
        CX2 = RenderSpec.MaxClip.X;
        CY2 = (i32)(
            0.5f + (Y2 + (CX2 - X2) * (Y1-Y2) / (f32)(X1-X2))
        );
    } break;

    case CC_NW:
    {
        CY2 = RenderSpec.MinClip.Y;
        CX2 = (i32)(
            0.5f + (X2 + (CY2 - Y2) * (X1-X2) / (f32)(Y1-Y2))
        );

        if (CX2 < RenderSpec.MinClip.X || CX2 > RenderSpec.MaxClip.X)
        {
            CX2 = RenderSpec.MinClip.X;
            CY2 = (i32)(
                0.5f + (Y2 + (CX2 - X2) * (Y1-Y2) / (f32)(X1-X2))
            );
        }
    } break;

    case CC_NE:
    {
        CY2 = RenderSpec.MinClip.Y;
        CX2 = (i32)(
            0.5f + (X2 + (CY2 - Y2) * (X1-X2) / (f32)(Y1-Y2))
        );

        if (CX2 < RenderSpec.MinClip.X || CX2 > RenderSpec.MaxClip.X)
        {
            CX2 = RenderSpec.MaxClip.X;
            CY2 = (i32)(
                0.5f + (Y2 + (CX2 - X2) * (Y1-Y2) / (f32)(X1-X2))
            );
        }
    } break;

    case CC_SW:
    {
        CY2 = RenderSpec.MaxClip.Y;
        CX2 = (i32)(
            0.5f + (X2 + (CY2 - Y2) * (X1-X2) / (f32)(Y1-Y2))
        );

        if (CX2 < RenderSpec.MinClip.X || CX2 > RenderSpec.MaxClip.X)
        {
            CX2 = RenderSpec.MinClip.X;
            CY2 = (i32)(
                0.5f + (Y2 + (CX2 - X2) * (Y1-Y2) / (f32)(X1-X2))
            );
        }
    } break;

    case CC_SE:
    {
        CY2 = RenderSpec.MaxClip.Y;
        CX2 = (i32)(
            0.5f + (X2 + (CY2 - Y2) * (X1-X2) / (f32)(Y1-Y2))
        );

        if (CX2 < RenderSpec.MinClip.X || CX2 > RenderSpec.MaxClip.X)
        {
            CX2 = RenderSpec.MaxClip.X;
            CY2 = (i32)(
                0.5f + (Y2 + (CX2 - X2) * (Y1-Y2) / (f32)(X1-X2))
            );
        }
    } break;

    default: {} break;
    }

    // Check if we still can't see the line
    if (CX1 < RenderSpec.MinClip.X || CX1 > RenderSpec.MaxClip.X ||
        CX2 < RenderSpec.MinClip.X || CX2 > RenderSpec.MaxClip.X ||
        CY1 < RenderSpec.MinClip.Y || CY1 > RenderSpec.MaxClip.Y ||
        CY2 < RenderSpec.MinClip.Y || CY2 > RenderSpec.MaxClip.Y)
    {
        return false;
    }

    X1 = CX1;
    X2 = CX2;
    Y1 = CY1;
    Y2 = CY2;

    return true;
}

#if 0 // Deprecated
void VRenderer::DrawTopTriangleInt(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, i32 X3, i32 Y3, u32 Color) const
{
    // Sort
    if (X1 > X2)
    {
        i32 Temp;
        VL_SWAP(X1, X2, Temp);
    }

    f32 XStart = (f32)X1;
    f32 XEnd = (f32)X2 + 0.5f;

    f32 OneDivHeight = 1.0f / (Y3 - Y1);
    f32 XDiffStart = (f32)(X3 - X1) * OneDivHeight;
    f32 XDiffEnd = (f32)(X3 - X2) * OneDivHeight;

    // Y clipping
    if (Y1 < RenderSpec.MinClip.Y)
    {
        XStart += XDiffStart * (RenderSpec.MinClip.Y - Y1);
        XEnd += XDiffEnd * (RenderSpec.MinClip.Y - Y1);
        Y1 = RenderSpec.MinClip.Y;
    }
    if (Y3 > RenderSpec.MaxClip.Y)
    {
        Y3 = RenderSpec.MaxClip.Y;
    }

    Buffer += Y1 * Pitch;

    // Test if we need X clipping
    if (X1 >= RenderSpec.MinClip.X && X1 <= RenderSpec.MaxClip.X &&
        X2 >= RenderSpec.MinClip.X && X2 <= RenderSpec.MaxClip.X &&
        X3 >= RenderSpec.MinClip.X && X3 <= RenderSpec.MaxClip.X)
    {
        for (i32 Y = Y1; Y <= Y3; ++Y)
        {
            Memory.MemSetQuad(Buffer + (u32)XStart, Color, (i32)((XEnd - XStart) + 1.0f));
            XStart += XDiffStart;
            XEnd += XDiffEnd;
            Buffer += Pitch;
        }
    }
    else
    {
        for (i32 Y = Y1; Y <= Y3; ++Y, Buffer += Pitch)
        {
            f32 XClippedStart = XStart;
            f32 XClippedEnd = XEnd;

            XStart += XDiffStart;
            XEnd += XDiffEnd;

            if (XClippedStart < RenderSpec.MinClipFloat.X)
            {
                if (XClippedEnd < RenderSpec.MinClipFloat.X)
                {
                    continue;
                }

                XClippedStart = RenderSpec.MinClipFloat.X;
            }

            if (XClippedEnd > RenderSpec.MaxClipFloat.X)
            {
                if (XClippedStart > RenderSpec.MaxClipFloat.X)
                {
                    continue;
                }

                XClippedEnd = RenderSpec.MaxClipFloat.X;
            }

            Memory.MemSetQuad(Buffer + (u32)XClippedStart, Color, ((i32)XClippedEnd - (i32)XClippedStart) + 1);
        }
    }
}

void VRenderer::DrawBottomTriangleInt(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, i32 X3, i32 Y3, u32 Color) const
{
    // Sort
    if (X2 > X3)
    {
        i32 Temp;
        VL_SWAP(X2, X3, Temp);
    }

    f32 XStart = (f32)X1;
    f32 XEnd = (f32)X1 + 0.5f;

    f32 OneDivHeight = 1.0f / (Y3 - Y1);
    f32 XDiffStart = (f32)(X2 - X1) * OneDivHeight;
    f32 XDiffEnd = (f32)(X3 - X1) * OneDivHeight;

    // Y clipping
    if (Y1 < RenderSpec.MinClip.Y)
    {
        XStart += XDiffStart * (RenderSpec.MinClip.Y - Y1);
        XEnd += XDiffEnd * (RenderSpec.MinClip.Y - Y1);
        Y1 = RenderSpec.MinClip.Y;
    }
    if (Y3 > RenderSpec.MaxClip.Y)
    {
        Y3 = RenderSpec.MaxClip.Y;
    }

    Buffer += Y1 * Pitch;

    // Test if we need X clipping
    if (X1 >= RenderSpec.MinClip.X && X1 <= RenderSpec.MaxClip.X &&
        X2 >= RenderSpec.MinClip.X && X2 <= RenderSpec.MaxClip.X &&
        X3 >= RenderSpec.MinClip.X && X3 <= RenderSpec.MaxClip.X)
    {
        for (i32 Y = Y1; Y <= Y3; ++Y)
        {
            Memory.MemSetQuad(Buffer + (u32)XStart, Color, (i32)((XEnd - XStart) + 1.0f));
            XStart += XDiffStart;
            XEnd += XDiffEnd;
            Buffer += Pitch;
        }
    }
    else
    {
        for (i32 Y = Y1; Y <= Y3; ++Y, Buffer += Pitch)
        {
            f32 XClippedStart = XStart;
            f32 XClippedEnd = XEnd;

            XStart += XDiffStart;
            XEnd += XDiffEnd;

            if (XClippedStart < RenderSpec.MinClipFloat.X)
            {
                if (XClippedEnd < RenderSpec.MinClipFloat.X)
                {
                    continue;
                }

                XClippedStart = RenderSpec.MinClipFloat.X;
            }

            if (XClippedEnd > RenderSpec.MaxClipFloat.X)
            {
                if (XClippedStart > RenderSpec.MaxClipFloat.X)
                {
                    continue;
                }

                XClippedEnd = RenderSpec.MaxClipFloat.X;
            }

            Memory.MemSetQuad(Buffer + (u32)XClippedStart, Color, ((i32)XClippedEnd - (i32)XClippedStart) + 1);
        }
    }
}

void VRenderer::DrawTriangleInt(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, i32 X3, i32 Y3, u32 Color) const
{
    // Test for vertical and horizontal triangle
    if ((X1 == X2 && X2 == X3) || (Y1 == Y2 && Y2 == Y3))
    {
        return;
    }

    // Do sorting by Y
    i32 Temp;
    if (Y1 > Y2)
    {
        VL_SWAP(Y1, Y2, Temp);
        VL_SWAP(X1, X2, Temp);
    }
    if (Y1 > Y3)
    {
        VL_SWAP(Y1, Y3, Temp);
        VL_SWAP(X1, X3, Temp);
    }
    if (Y2 > Y3)
    {
        VL_SWAP(Y2, Y3, Temp);
        VL_SWAP(X2, X3, Temp);
    }

    // Whole clipping test
    if (Y3 < RenderSpec.MinClip.Y || Y1 > RenderSpec.MaxClip.Y ||
        (X1 < RenderSpec.MinClip.X && X2 < RenderSpec.MinClip.X && X3 < RenderSpec.MinClip.X) ||
        (X1 > RenderSpec.MaxClip.X && X2 > RenderSpec.MaxClip.X && X3 > RenderSpec.MaxClip.X))
    {
        return;
    }

    if (Y1 == Y2)
    {
        DrawTopTriangleInt(Buffer, Pitch, X1, Y1, X2, Y2, X3, Y3, Color);
    }
    else if (Y2 == Y3)
    {
        DrawBottomTriangleInt(Buffer, Pitch, X1, Y1, X2, Y2, X3, Y3, Color);
    }
    else
    {
        // Find NewX which will divide triangle on Bottom/Top parts
        i32 NewX = X1 + (i32)(0.5f + (f32)(Y2 - Y1) * ((f32)(X3 - X1) / (f32)(Y3 - Y1)) );

        DrawBottomTriangleInt(Buffer, Pitch, X1, Y1, NewX, Y2, X2, Y2, Color);
        DrawTopTriangleInt(Buffer, Pitch, X2, Y2, NewX, Y2, X3, Y3, Color);
    }
}

void VRenderer::DrawTopTriangleFloat(u32* Buffer, i32 Pitch, f32 X1, f32 Y1, f32 X2, f32 Y2, f32 X3, f32 Y3, u32 Color) const
{
    // Sort
    if (X2 < X1)
    {
        f32 Temp;
        VL_SWAP(X1, X2, Temp);
    }

    // Set Start End vectors
    f32 XStart = X1, XEnd = X2;
    i32f YStart, YEnd;

    // Compute deltas
    f32 Height = Y3 - Y1;
    f32 XDeltaStart = (X3 - X1) / Height;
    f32 XDeltaEnd   = (X3 - X2) / Height;

    if (Y1 < RenderSpec.MinClipFloat.Y)
    {
        YStart = RenderSpec.MinClip.Y;

        f32 YDiff = (f32)YStart - Y1;
        XStart += YDiff * XDeltaStart;
        XEnd   += YDiff * XDeltaEnd;
    }
    else
    {
        YStart = (i32f)Math.Ceil(Y1);

        f32 YDiff = (f32)YStart - Y1;
        XStart += YDiff * XDeltaStart;
        XEnd   += YDiff * XDeltaEnd;
    }

    if (Y3 > RenderSpec.MaxClipFloat.Y)
    {
        YEnd = RenderSpec.MaxClip.Y;
    }
    else
    {
        YEnd = (i32f)Math.Ceil(Y3) - 1;
    }

    Buffer += Pitch * YStart;

    // Test if we don't need X clipping
    if ((X1 >= RenderSpec.MinClip.X && X1 <= RenderSpec.MaxClip.X) &&
        (X2 >= RenderSpec.MinClip.X && X2 <= RenderSpec.MaxClip.X) &&
        (X3 >= RenderSpec.MinClip.X && X3 <= RenderSpec.MaxClip.X))
    {
        for (i32f Y = YStart; Y <= YEnd; ++Y, Buffer += Pitch)
        {
            Memory.MemSetQuad(Buffer + (u32)XStart, Color, (VSizeType)(XEnd - XStart) + 1);
            XStart += XDeltaStart;
            XEnd   += XDeltaEnd;
        }
    }
    else
    {
        for (i32f Y = YStart; Y <= YEnd; ++Y, Buffer += Pitch)
        {
            f32 XClippedStart = XStart;
            f32 XClippedEnd = XEnd;

            XStart += XDeltaStart;
            XEnd   += XDeltaEnd;

            if (XClippedStart < RenderSpec.MinClipFloat.X)
            {
                if (XClippedEnd < RenderSpec.MinClipFloat.X)
                {
                    continue;
                }
                XClippedStart = RenderSpec.MinClipFloat.X;
            }

            if (XClippedEnd > RenderSpec.MaxClipFloat.X)
            {
                if (XClippedStart > RenderSpec.MaxClipFloat.X)
                {
                    continue;
                }
                XClippedEnd = RenderSpec.MaxClipFloat.X;
            }

            Memory.MemSetQuad(Buffer + (u32)XClippedStart, Color, (VSizeType)(XClippedEnd - XClippedStart) + 1);
        }
    }
}
void VRenderer::DrawBottomTriangleFloat(u32* Buffer, i32 Pitch, f32 X1, f32 Y1, f32 X2, f32 Y2, f32 X3, f32 Y3, u32 Color) const
{
    // Sort
    if (X3 < X2)
    {
        f32 Temp;
        VL_SWAP(X2, X3, Temp);
    }

    // Set Start End vectors
    f32 XStart = X1, XEnd = X1;
    i32f YStart, YEnd;

    // Compute deltas
    f32 Height = Y3 - Y1;
    f32 XDeltaStart = (X2 - X1) / Height;
    f32 XDeltaEnd   = (X3 - X1) / Height;

    if (Y1 < RenderSpec.MinClipFloat.Y)
    {
        YStart = RenderSpec.MinClip.Y;

        f32 YDiff = (f32)YStart - Y1;
        XStart += YDiff * XDeltaStart;
        XEnd   += YDiff * XDeltaEnd;
    }
    else
    {
        YStart = (i32f)Math.Ceil(Y1);

        f32 YDiff = (f32)YStart - Y1;
        XStart += YDiff * XDeltaStart;
        XEnd   += YDiff * XDeltaEnd;
    }

    if (Y3 > RenderSpec.MaxClipFloat.Y)
    {
        YEnd = RenderSpec.MaxClip.Y;
    }
    else
    {
        YEnd = (i32f)Math.Ceil(Y3) - 1;
    }

    Buffer += Pitch * YStart;

    // Test if we don't need X clipping
    if ((X1 >= RenderSpec.MinClip.X && X1 <= RenderSpec.MaxClip.X) &&
        (X2 >= RenderSpec.MinClip.X && X2 <= RenderSpec.MaxClip.X) &&
        (X3 >= RenderSpec.MinClip.X && X3 <= RenderSpec.MaxClip.X))
    {
        for (i32f Y = YStart; Y <= YEnd; ++Y, Buffer += Pitch)
        {
            Memory.MemSetQuad(Buffer + (u32)XStart, Color, (VSizeType)(XEnd - XStart) + 1);
            XStart += XDeltaStart;
            XEnd   += XDeltaEnd;
        }
    }
    else
    {
        for (i32f Y = YStart; Y <= YEnd; ++Y, Buffer += Pitch)
        {
            f32 XClippedStart = XStart;
            f32 XClippedEnd = XEnd;

            XStart += XDeltaStart;
            XEnd   += XDeltaEnd;

            if (XClippedStart < RenderSpec.MinClipFloat.X)
            {
                if (XClippedEnd < RenderSpec.MinClipFloat.X)
                {
                    continue;
                }
                XClippedStart = RenderSpec.MinClipFloat.X;
            }

            if (XClippedEnd > RenderSpec.MaxClipFloat.X)
            {
                if (XClippedStart > RenderSpec.MaxClipFloat.X)
                {
                    continue;
                }
                XClippedEnd = RenderSpec.MaxClipFloat.X;
            }

            Memory.MemSetQuad(Buffer + (u32)XClippedStart, Color, (VSizeType)(XClippedEnd - XClippedStart) + 1);
        }
    }
}
void VRenderer::DrawTriangleFloat(u32* Buffer, i32 Pitch, f32 X1, f32 Y1, f32 X2, f32 Y2, f32 X3, f32 Y3, u32 Color) const
{
    // Vertical, horizontal triangle clipping
    if ((Math.IsEqualFloat(X1, X2) && Math.IsEqualFloat(X2, X3)) ||
        (Math.IsEqualFloat(Y1, Y2) && Math.IsEqualFloat(Y2, Y3)))
    {
        return;
    }

    // Sort by Y
    if (Y2 < Y1)
    {
        f32 Temp;
        VL_SWAP(X1, X2, Temp);
        VL_SWAP(Y1, Y2, Temp);
    }
    if (Y3 < Y1)
    {
        f32 Temp;
        VL_SWAP(X1, X3, Temp);
        VL_SWAP(Y1, Y3, Temp);
    }
    if (Y3 < Y2)
    {
        f32 Temp;
        VL_SWAP(X2, X3, Temp);
        VL_SWAP(Y2, Y3, Temp);
    }

    // Screen space clipping
    if ((Y3 < RenderSpec.MinClipFloat.Y || Y1 > RenderSpec.MaxClipFloat.Y) ||
        (X1 < RenderSpec.MinClipFloat.X && X2 < RenderSpec.MinClipFloat.X && X3 < RenderSpec.MinClipFloat.X) ||
        (X1 > RenderSpec.MaxClipFloat.X && X2 > RenderSpec.MaxClipFloat.X && X3 > RenderSpec.MaxClipFloat.X))
    {
        return;
    }

    if (Math.IsEqualFloat(Y1, Y2))
    {
        DrawTopTriangleFloat(Buffer, Pitch, X1, Y1, X2, Y2, X3, Y3, Color);
    }
    else if (Math.IsEqualFloat(Y2, Y3))
    {
        DrawBottomTriangleFloat(Buffer, Pitch, X1, Y1, X2, Y2, X3, Y3, Color);
    }
    else
    {
        f32 NewX = X1 + (Y2 - Y1) * ((X3 - X1) / (Y3 - Y1));
        DrawBottomTriangleFloat(Buffer, Pitch, X1, Y1, X2, Y2, NewX, Y2, Color);
        DrawTopTriangleFloat(Buffer, Pitch, X2, Y2, NewX, Y2, X3, Y3, Color);
    }
}

void VRenderer::DrawFlatTriangle(u32* Buffer, i32 Pitch, const VPolyFace& Poly) const
{
    enum class ETriangleCase
    {
        Top,
        Bottom,
        General
    };

    i32 V0 = 0, V1 = 1, V2 = 2;

    // Sort by Y
    i32 TempInt;
    if (Poly.TransVtx[V1].Y < Poly.TransVtx[V0].Y)
    {
        VL_SWAP(V0, V1, TempInt);
    }
    if (Poly.TransVtx[V2].Y < Poly.TransVtx[V0].Y)
    {
        VL_SWAP(V0, V2, TempInt);
    }
    if (Poly.TransVtx[V2].Y < Poly.TransVtx[V1].Y)
    {
        VL_SWAP(V1, V2, TempInt);
    }

    // Test if we can't see it
    if (Poly.TransVtx[V2].Y < RenderSpec.MinClipFloat.Y ||
        Poly.TransVtx[V0].Y > RenderSpec.MaxClipFloat.Y ||
        (Poly.TransVtx[V0].X < RenderSpec.MinClipFloat.X && Poly.TransVtx[V1].X < RenderSpec.MinClipFloat.X && Poly.TransVtx[V2].X < RenderSpec.MinClipFloat.X) ||
        (Poly.TransVtx[V0].X > RenderSpec.MaxClipFloat.X && Poly.TransVtx[V1].X > RenderSpec.MaxClipFloat.X && Poly.TransVtx[V2].X > RenderSpec.MaxClipFloat.X))
    {
        return;
    }

    // Convert Y to integers
    i32 Y0 = (i32)(Poly.TransVtx[V0].Y + 0.5f);
    i32 Y1 = (i32)(Poly.TransVtx[V1].Y + 0.5f);
    i32 Y2 = (i32)(Poly.TransVtx[V2].Y + 0.5f);

    // Found triangle case and sort vertices by X
    ETriangleCase TriangleCase;
    if (Y0 == Y1)
    {
        if (Poly.TransVtx[V1].X < Poly.TransVtx[V0].X)
        {
            VL_SWAP(V0, V1, TempInt);
            VL_SWAP(Y0, Y1, TempInt);
        }
        TriangleCase = ETriangleCase::Top;
    }
    else if (Y1 == Y2)
    {
        if (Poly.TransVtx[V2].X < Poly.TransVtx[V1].X)
        {
            VL_SWAP(V1, V2, TempInt);
            VL_SWAP(Y1, Y2, TempInt);
        }
        TriangleCase = ETriangleCase::Bottom;
    }
    else
    {
        TriangleCase = ETriangleCase::General;
    }

    // Convert coords to integer
    i32 X0 = (i32)(Poly.TransVtx[V0].X + 0.5f);
    i32 X1 = (i32)(Poly.TransVtx[V1].X + 0.5f);
    i32 X2 = (i32)(Poly.TransVtx[V2].X + 0.5f);

    // Vertical, horizontal triangle test
    if ((Y0 == Y1 && Y1 == Y2) || (X0 == X1 && X1 == X2))
    {
        return;
    }

    i32 YStart;
    i32 YEnd;
    u32 Color = Poly.LitColor[0];

    i32 ZVtx0 = (i32)(Poly.TransVtx[V0].Z + 0.5f);
    i32 ZVtx1 = (i32)(Poly.TransVtx[V1].Z + 0.5f);
    i32 ZVtx2 = (i32)(Poly.TransVtx[V2].Z + 0.5f);

    // Fixed coords, color channels for rasterization
    fx16 XLeft;
    fx16 XRight;
    fx16 ZLeft, ZRight;

    // Coords, colors fixed deltas by Y
    fx16 XDeltaLeftByY;
    fx16 ZDeltaLeftByY;

    fx16 XDeltaRightByY;
    fx16 ZDeltaRightByY;

    fx16* ZBufferArray;

    if (TriangleCase == ETriangleCase::Top ||
        TriangleCase == ETriangleCase::Bottom)
    {
        i32 YDiff = Y2 - Y0;

        if (TriangleCase == ETriangleCase::Top)
        {
            // Compute deltas for coords, colors
            XDeltaLeftByY = IntToFx16(X2 - X0) / YDiff;
            ZDeltaLeftByY = IntToFx16(ZVtx2 - ZVtx0) / YDiff;

            XDeltaRightByY = IntToFx16(X2 - X1) / YDiff;
            ZDeltaRightByY = IntToFx16(ZVtx2 - ZVtx1) / YDiff;

            // Clipping Y
            if (Y0 < RenderSpec.MinClip.Y)
            {
                YDiff = RenderSpec.MinClip.Y - Y0;
                YStart = RenderSpec.MinClip.Y;

                XLeft = IntToFx16(X0) + YDiff * XDeltaLeftByY;
                ZLeft = IntToFx16(ZVtx0) + YDiff * ZDeltaLeftByY;

                XRight = IntToFx16(X1) + YDiff * XDeltaRightByY;
                ZRight = IntToFx16(ZVtx1) + YDiff * ZDeltaRightByY;
            }
            else
            {
                YStart = Y0;

                XLeft = IntToFx16(X0);
                ZLeft = IntToFx16(ZVtx0);

                XRight = IntToFx16(X1);
                ZRight = IntToFx16(ZVtx1);
            }
        }
        else // Bottom case
        {
            // Compute deltas for coords, colors
            XDeltaLeftByY = IntToFx16(X1 - X0) / YDiff;
            ZDeltaLeftByY = IntToFx16(ZVtx1 - ZVtx0) / YDiff;

            XDeltaRightByY = IntToFx16(X2 - X0) / YDiff;
            ZDeltaRightByY = IntToFx16(ZVtx2 - ZVtx0) / YDiff;

            // Clipping Y
            if (Y0 < RenderSpec.MinClip.Y)
            {
                YDiff = RenderSpec.MinClip.Y - Y0;
                YStart = RenderSpec.MinClip.Y;

                XLeft = IntToFx16(X0) + YDiff * XDeltaLeftByY;
                ZLeft = IntToFx16(ZVtx0) + YDiff * ZDeltaLeftByY;

                XRight = IntToFx16(X0) + YDiff * XDeltaRightByY;
                ZRight = IntToFx16(ZVtx0) + YDiff * ZDeltaRightByY;
            }
            else
            {
                YStart = Y0;

                XLeft = IntToFx16(X0);
                ZLeft = IntToFx16(ZVtx0);

                XRight = IntToFx16(X0);
                ZRight = IntToFx16(ZVtx0);
            }
        }

        // Clip bottom Y
        if (Y2 > RenderSpec.MaxClip.Y)
        {
            YEnd = RenderSpec.MaxClip.Y;
        }
        else
        {
            YEnd = Y2;
        }

        // Test for clipping X
        if (X0 < RenderSpec.MinClip.X || X1 < RenderSpec.MinClip.X || X2 < RenderSpec.MinClip.X ||
            X0 > RenderSpec.MaxClip.X || X1 > RenderSpec.MaxClip.X || X2 > RenderSpec.MaxClip.X)
        {
            // Align buffer pointer
            Buffer += Pitch * YStart;
            ZBufferArray = (fx16*)ZBuffer.Buffer + (ZBuffer.Pitch * YStart);

            // Process each Y
            for (i32f Y = YStart; Y <= YEnd; ++Y)
            {
                // Compute starting values
                i32f XStart = Fx16ToIntRounded(XLeft);
                i32f XEnd = Fx16ToIntRounded(XRight);

                fx16 Z = ZLeft;

                // Compute deltas for X interpolation
                i32f XDiff = XEnd - XStart;

                fx16 ZDeltaByX;
                if (XDiff > 0)
                {
                    ZDeltaByX = (ZRight - ZLeft) / XDiff;
                }
                else
                {
                    ZDeltaByX = (ZRight - ZLeft);
                }

                // X clipping
                if (XStart < RenderSpec.MinClip.X)
                {
                    i32 XDiff = RenderSpec.MinClip.X - XStart;
                    XStart = RenderSpec.MinClip.X;

                    Z += XDiff * ZDeltaByX;
                }
                if (XEnd > RenderSpec.MaxClip.X)
                {
                    XEnd = RenderSpec.MaxClip.X;
                }

                // Process each X
                for (i32f X = XStart; X <= XEnd; ++X)
                {
                    if (Z < ZBufferArray[X])
                    {
                        Buffer[X] = Color;

                        ZBufferArray[X] = Z;
                    }

                    // Update X values
                    Z += ZDeltaByX;
                }

                // Update Y values
                XLeft += XDeltaLeftByY;
                ZLeft += ZDeltaLeftByY;

                XRight += XDeltaRightByY;
                ZRight += ZDeltaRightByY;

                Buffer += Pitch;
                ZBufferArray += ZBuffer.Pitch;
            }
        }
        else // Non-clipped version
        {
            // Align buffer pointer
            Buffer += Pitch * YStart;
            ZBufferArray = (fx16*)ZBuffer.Buffer + (ZBuffer.Pitch * YStart);

            // Process each Y
            for (i32f Y = YStart; Y <= YEnd; ++Y)
            {
                // Compute starting values
                i32f XStart = Fx16ToIntRounded(XLeft);
                i32f XEnd = Fx16ToIntRounded(XRight);

                fx16 Z = ZLeft;

                // Compute deltas for X interpolation
                i32f XDiff = XEnd - XStart;

                fx16 ZDeltaByX;
                if (XDiff > 0)
                {
                    ZDeltaByX = (ZRight - ZLeft) / XDiff;
                }
                else
                {
                    ZDeltaByX = (ZRight - ZLeft);
                }

                // Process each X
                for (i32f X = XStart; X <= XEnd; ++X)
                {
                   if (Z < ZBufferArray[X])
                    {
                        Buffer[X] = Color;

                        ZBufferArray[X] = Z;
                    }

                    // Update X values
                    Z += ZDeltaByX;
                }

                // Update Y values
                XLeft += XDeltaLeftByY;
                ZLeft += ZDeltaLeftByY;

                XRight += XDeltaRightByY;
                ZRight += ZDeltaRightByY;

                Buffer += Pitch;
                ZBufferArray += ZBuffer.Pitch;
            }
        }
    }
    else // General case
    {
        b32 bRestartInterpolationAtLeftHand = true;
        i32 YRestartInterpolation = Y1;

        // Clip bottom Y
        if (Y2 > RenderSpec.MaxClip.Y)
        {
            YEnd = RenderSpec.MaxClip.Y;
        }
        else
        {
            YEnd = Y2;
        }

        // Clip top Y
        if (Y1 < RenderSpec.MinClip.Y)
        {
            // Compute deltas
            i32 YDiffLeft = (Y2 - Y1);
            XDeltaLeftByY = IntToFx16(X2 - X1) / YDiffLeft;
            ZDeltaLeftByY = IntToFx16(ZVtx2 - ZVtx1) / YDiffLeft;

            i32 YDiffRight = (Y2 - Y0);
            XDeltaRightByY = IntToFx16(X2 - X0) / YDiffRight;
            ZDeltaRightByY = IntToFx16(ZVtx2 - ZVtx0) / YDiffRight;

            // Do clipping
            YDiffLeft = (RenderSpec.MinClip.Y - Y1);
            XLeft = IntToFx16(X1) + YDiffLeft * XDeltaLeftByY;
            ZLeft = IntToFx16(ZVtx1) + YDiffLeft * ZDeltaLeftByY;

            YDiffRight = (RenderSpec.MinClip.Y - Y0);
            XRight = IntToFx16(X0) + YDiffRight * XDeltaRightByY;
            ZRight = IntToFx16(ZVtx0) + YDiffRight * ZDeltaRightByY;

            YStart = RenderSpec.MinClip.Y;

            /* NOTE(sean):
                Test if we need swap to keep rendering left to right.
                It can happen because we assume that
                Y1 is on left hand side and Y2 on right.
             */
            if (XDeltaRightByY > XDeltaLeftByY)
            {
                VL_SWAP(XDeltaLeftByY, XDeltaRightByY, TempInt);
                VL_SWAP(ZDeltaLeftByY, ZDeltaRightByY, TempInt);

                VL_SWAP(XLeft, XRight, TempInt);
                VL_SWAP(ZLeft, ZRight, TempInt);

                VL_SWAP(X1, X2, TempInt);
                VL_SWAP(Y1, Y2, TempInt);
                VL_SWAP(ZVtx1, ZVtx2, TempInt);

                bRestartInterpolationAtLeftHand = false; // Restart at right hand side
            }
        }
        else if (Y0 < RenderSpec.MinClip.Y)
        {
            i32 YDiffLeft = (Y1 - Y0);
            XDeltaLeftByY = IntToFx16(X1 - X0) / YDiffLeft;
            ZDeltaLeftByY = IntToFx16(ZVtx1 - ZVtx0) / YDiffLeft;

            i32 YDiffRight = (Y2 - Y0);
            XDeltaRightByY = IntToFx16(X2 - X0) / YDiffRight;
            ZDeltaRightByY = IntToFx16(ZVtx2 - ZVtx0) / YDiffRight;

            i32 YDiff = (RenderSpec.MinClip.Y - Y0);
            XLeft = IntToFx16(X0) + YDiff * XDeltaLeftByY;
            ZLeft = IntToFx16(ZVtx0) + YDiff * ZDeltaLeftByY;

            XRight = IntToFx16(X0) + YDiff * XDeltaRightByY;
            ZRight = IntToFx16(ZVtx0) + YDiff * ZDeltaRightByY;

            YStart = RenderSpec.MinClip.Y;

            /* NOTE(sean):
                Test if we need swap to keep rendering left to right.
                It can happen because we assume that
                Y1 is on left hand side and Y2 on right.
             */
            if (XDeltaRightByY < XDeltaLeftByY)
            {
                VL_SWAP(XDeltaLeftByY, XDeltaRightByY, TempInt);
                VL_SWAP(ZDeltaLeftByY, ZDeltaRightByY, TempInt);

                VL_SWAP(XLeft, XRight, TempInt);
                VL_SWAP(ZLeft, ZRight, TempInt);

                VL_SWAP(X1, X2, TempInt);
                VL_SWAP(Y1, Y2, TempInt);
                VL_SWAP(ZVtx1, ZVtx2, TempInt);

                bRestartInterpolationAtLeftHand = false; // Restart at right hand side
            }
        }
        else // No top Y clipping
        {
            i32 YDiffLeft = (Y1 - Y0);
            XDeltaLeftByY = IntToFx16(X1 - X0) / YDiffLeft;
            ZDeltaLeftByY = IntToFx16(ZVtx1 - ZVtx0) / YDiffLeft;

            i32 YDiffRight = (Y2 - Y0);
            XDeltaRightByY = IntToFx16(X2 - X0) / YDiffRight;
            ZDeltaRightByY = IntToFx16(ZVtx2 - ZVtx0) / YDiffRight;

            XRight = XLeft = IntToFx16(X0);
            ZRight = ZLeft = IntToFx16(ZVtx0);

            YStart = Y0;

            /* NOTE(sean):
                Test if we need swap to keep rendering left to right.
                It can happen because we assume that
                Y1 is on left hand side and Y2 on right.
             */
            // TODO(sean): Test if we can simplify it
            if (XDeltaRightByY < XDeltaLeftByY)
            {
                VL_SWAP(XDeltaLeftByY, XDeltaRightByY, TempInt);
                VL_SWAP(ZDeltaLeftByY, ZDeltaRightByY, TempInt);

                VL_SWAP(XLeft, XRight, TempInt);
                VL_SWAP(ZLeft, ZRight, TempInt);

                VL_SWAP(X1, X2, TempInt);
                VL_SWAP(Y1, Y2, TempInt);
                VL_SWAP(ZVtx1, ZVtx2, TempInt);

                bRestartInterpolationAtLeftHand = false; // Restart at right hand side
            }
        }

        // Test for clipping X
        if (X0 < RenderSpec.MinClip.X || X1 < RenderSpec.MinClip.X || X2 < RenderSpec.MinClip.X ||
            X0 > RenderSpec.MaxClip.X || X1 > RenderSpec.MaxClip.X || X2 > RenderSpec.MaxClip.X)
        {
            // Align video buffer
            Buffer += Pitch * YStart;
            ZBufferArray = (fx16*)ZBuffer.Buffer + (ZBuffer.Pitch * YStart);

            // Process each Y
            for (i32f Y = YStart; Y <= YEnd; ++Y)
            {
                // Compute initial values
                i32 XStart = Fx16ToIntRounded(XLeft);
                i32 XEnd   = Fx16ToIntRounded(XRight);

                fx16 Z = ZLeft;

                // Compute interpolants
                fx16 ZDeltaByX;

                i32 XDiff = XEnd - XStart;
                if (XDiff > 0)
                {
                    ZDeltaByX = (ZRight - ZLeft) / XDiff;
                }
                else
                {
                    ZDeltaByX = (ZRight - ZLeft);
                }

                // Test if we need clipping
                if (XStart < RenderSpec.MinClip.X)
                {
                    XDiff = RenderSpec.MinClip.X - XStart;
                    XStart = RenderSpec.MinClip.X;

                    Z += ZDeltaByX * XDiff;
                }
                if (XEnd > RenderSpec.MaxClip.X)
                {
                    XEnd = RenderSpec.MaxClip.X;
                }

                // Process each X
                for (i32f X = XStart; X <= XEnd; ++X)
                {
                   if (Z < ZBufferArray[X])
                    {
                        Buffer[X] = Color;

                        ZBufferArray[X] = Z;
                    }

                    Z += ZDeltaByX;
                }

                // Update values those change along Y
                XLeft += XDeltaLeftByY;
                ZLeft += ZDeltaLeftByY;

                XRight += XDeltaRightByY;
                ZRight += ZDeltaRightByY;

                Buffer += Pitch;
                ZBufferArray += ZBuffer.Pitch;

                // Test for changing interpolant
                if (Y == YRestartInterpolation)
                {
                    if (bRestartInterpolationAtLeftHand)
                    {
                        // Compute new values to get from Y1 to Y2
                        i32 YDiff = (Y2 - Y1);

                        XDeltaLeftByY = IntToFx16(X2 - X1) / YDiff;
                        ZDeltaLeftByY = IntToFx16(ZVtx2 - ZVtx1) / YDiff;

                        XLeft = IntToFx16(X1);
                        ZLeft = IntToFx16(ZVtx1);

                        // Align down on 1 Y
                        XLeft += XDeltaLeftByY;
                        ZLeft += ZDeltaLeftByY;
                    }
                    else
                    {
                        // Compute new values to get from Y2 to Y1 because we swapped them
                        i32 YDiff = (Y1 - Y2);

                        XDeltaRightByY = IntToFx16(X1 - X2) / YDiff;
                        ZDeltaRightByY = IntToFx16(ZVtx1 - ZVtx2) / YDiff;

                        XRight = IntToFx16(X2);
                        ZRight = IntToFx16(ZVtx2);

                        // Align down on 1 Y
                        XRight += XDeltaRightByY;
                        ZRight += ZDeltaRightByY;
                    }
                }
            }
        }
        else // No X clipping
        {
            // Align video buffer
            Buffer += Pitch * YStart;
            ZBufferArray = (fx16*)ZBuffer.Buffer + (ZBuffer.Pitch * YStart);

            // Process each Y
            for (i32f Y = YStart; Y <= YEnd; ++Y)
            {
                // Compute initial values
                i32 XStart = Fx16ToIntRounded(XLeft);
                i32 XEnd   = Fx16ToIntRounded(XRight);

                fx16 Z = ZLeft;

                // Compute interpolants
                fx16 ZDeltaByX;

                i32 XDiff = XEnd - XStart;
                if (XDiff > 0)
                {
                    ZDeltaByX = (ZRight - ZLeft) / XDiff;
                }
                else
                {
                    ZDeltaByX = (ZRight - ZLeft);
                }

                // Process each X
                for (i32f X = XStart; X <= XEnd; ++X)
                {
                   if (Z < ZBufferArray[X])
                    {
                        Buffer[X] = Color;

                        ZBufferArray[X] = Z;
                    }

                    Z += ZDeltaByX;
                }

                // Update values those change along Y
                XLeft += XDeltaLeftByY;
                ZLeft += ZDeltaLeftByY;

                XRight += XDeltaRightByY;
                ZRight += ZDeltaRightByY;

                Buffer += Pitch;
                ZBufferArray += ZBuffer.Pitch;

                // Test for changing interpolant
                if (Y == YRestartInterpolation)
                {
                    if (bRestartInterpolationAtLeftHand)
                    {
                        // Compute new values to get from Y1 to Y2
                        i32 YDiff = (Y2 - Y1);

                        XDeltaLeftByY = IntToFx16(X2 - X1) / YDiff;
                        ZDeltaLeftByY = IntToFx16(ZVtx2 - ZVtx1) / YDiff;

                        XLeft = IntToFx16(X1);
                        ZLeft = IntToFx16(ZVtx1);

                        // Align down on 1 Y
                        XLeft += XDeltaLeftByY;
                        ZLeft += ZDeltaLeftByY;
                    }
                    else
                    {
                        // Compute new values to get from Y2 to Y1 because we swapped them
                        i32 YDiff = (Y1 - Y2);

                        XDeltaRightByY = IntToFx16(X1 - X2) / YDiff;
                        ZDeltaRightByY = IntToFx16(ZVtx1 - ZVtx2) / YDiff;

                        XRight = IntToFx16(X2);
                        ZRight = IntToFx16(ZVtx2);

                        // Align down on 1 Y
                        XRight += XDeltaRightByY;
                        ZRight += ZDeltaRightByY;
                    }
                }
            }
        }
    }
}

void VRenderer::DrawGouraudTriangle(u32* Buffer, i32 Pitch, const VPolyFace& Poly) const
{
    enum class ETriangleCase
    {
        Top,
        Bottom,
        General
    };

    i32 V0 = 0, V1 = 1, V2 = 2;

    // Sort by Y
    i32 TempInt;
    if (Poly.TransVtx[V1].Y < Poly.TransVtx[V0].Y)
    {
        VL_SWAP(V0, V1, TempInt);
    }
    if (Poly.TransVtx[V2].Y < Poly.TransVtx[V0].Y)
    {
        VL_SWAP(V0, V2, TempInt);
    }
    if (Poly.TransVtx[V2].Y < Poly.TransVtx[V1].Y)
    {
        VL_SWAP(V1, V2, TempInt);
    }

    // Test if we can't see it
    if (Poly.TransVtx[V2].Y < RenderSpec.MinClipFloat.Y ||
        Poly.TransVtx[V0].Y > RenderSpec.MaxClipFloat.Y ||
        (Poly.TransVtx[V0].X < RenderSpec.MinClipFloat.X && Poly.TransVtx[V1].X < RenderSpec.MinClipFloat.X && Poly.TransVtx[V2].X < RenderSpec.MinClipFloat.X) ||
        (Poly.TransVtx[V0].X > RenderSpec.MaxClipFloat.X && Poly.TransVtx[V1].X > RenderSpec.MaxClipFloat.X && Poly.TransVtx[V2].X > RenderSpec.MaxClipFloat.X))
    {
        return;
    }

    // Convert Y to integers
    i32 Y0 = (i32)(Poly.TransVtx[V0].Y + 0.5f);
    i32 Y1 = (i32)(Poly.TransVtx[V1].Y + 0.5f);
    i32 Y2 = (i32)(Poly.TransVtx[V2].Y + 0.5f);

    // Found triangle case and sort vertices by X
    ETriangleCase TriangleCase;
    if (Y0 == Y1)
    {
        if (Poly.TransVtx[V1].X < Poly.TransVtx[V0].X)
        {
            VL_SWAP(V0, V1, TempInt);
            VL_SWAP(Y0, Y1, TempInt);
        }
        TriangleCase = ETriangleCase::Top;
    }
    else if (Y1 == Y2)
    {
        if (Poly.TransVtx[V2].X < Poly.TransVtx[V1].X)
        {
            VL_SWAP(V1, V2, TempInt);
            VL_SWAP(Y1, Y2, TempInt);
        }
        TriangleCase = ETriangleCase::Bottom;
    }
    else
    {
        TriangleCase = ETriangleCase::General;
    }

    // Convert coords to integer
    i32 X0 = (i32)(Poly.TransVtx[V0].X + 0.5f);
    i32 X1 = (i32)(Poly.TransVtx[V1].X + 0.5f);
    i32 X2 = (i32)(Poly.TransVtx[V2].X + 0.5f);

    // Vertical, horizontal triangle test
    if ((Y0 == Y1 && Y1 == Y2) || (X0 == X1 && X1 == X2))
    {
        return;
    }

    i32 YStart;
    i32 YEnd;

    i32 RVtx0 = Poly.LitColor[V0].R, GVtx0 = Poly.LitColor[V0].G, BVtx0 = Poly.LitColor[V0].B;
    i32 RVtx1 = Poly.LitColor[V1].R, GVtx1 = Poly.LitColor[V1].G, BVtx1 = Poly.LitColor[V1].B;
    i32 RVtx2 = Poly.LitColor[V2].R, GVtx2 = Poly.LitColor[V2].G, BVtx2 = Poly.LitColor[V2].B;

    i32 ZVtx0 = (i32)(Poly.TransVtx[V0].Z + 0.5f);
    i32 ZVtx1 = (i32)(Poly.TransVtx[V1].Z + 0.5f);
    i32 ZVtx2 = (i32)(Poly.TransVtx[V2].Z + 0.5f);

    // Fixed coords, color channels for rasterization
    fx16 XLeft, RLeft, GLeft, BLeft;
    fx16 XRight, RRight, GRight, BRight;
    fx16 ZLeft, ZRight;

    // Coords, colors fixed deltas by Y
    fx16 XDeltaLeftByY;
    fx16 RDeltaLeftByY, GDeltaLeftByY, BDeltaLeftByY;
    fx16 ZDeltaLeftByY;

    fx16 XDeltaRightByY;
    fx16 RDeltaRightByY, GDeltaRightByY, BDeltaRightByY;
    fx16 ZDeltaRightByY;

    fx16* ZBufferArray;

    if (TriangleCase == ETriangleCase::Top ||
        TriangleCase == ETriangleCase::Bottom)
    {
        i32 YDiff = Y2 - Y0;

        if (TriangleCase == ETriangleCase::Top)
        {
            // Compute deltas for coords, colors
            XDeltaLeftByY = IntToFx16(X2 - X0) / YDiff;
            RDeltaLeftByY = IntToFx16(RVtx2 - RVtx0) / YDiff;
            GDeltaLeftByY = IntToFx16(GVtx2 - GVtx0) / YDiff;
            BDeltaLeftByY = IntToFx16(BVtx2 - BVtx0) / YDiff;
            ZDeltaLeftByY = IntToFx16(ZVtx2 - ZVtx0) / YDiff;

            XDeltaRightByY = IntToFx16(X2 - X1) / YDiff;
            RDeltaRightByY = IntToFx16(RVtx2 - RVtx1) / YDiff;
            GDeltaRightByY = IntToFx16(GVtx2 - GVtx1) / YDiff;
            BDeltaRightByY = IntToFx16(BVtx2 - BVtx1) / YDiff;
            ZDeltaRightByY = IntToFx16(ZVtx2 - ZVtx1) / YDiff;

            // Clipping Y
            if (Y0 < RenderSpec.MinClip.Y)
            {
                YDiff = RenderSpec.MinClip.Y - Y0;
                YStart = RenderSpec.MinClip.Y;

                XLeft = IntToFx16(X0) + YDiff * XDeltaLeftByY;
                RLeft = IntToFx16(RVtx0) + YDiff * RDeltaLeftByY;
                GLeft = IntToFx16(GVtx0) + YDiff * GDeltaLeftByY;
                BLeft = IntToFx16(BVtx0) + YDiff * BDeltaLeftByY;
                ZLeft = IntToFx16(ZVtx0) + YDiff * ZDeltaLeftByY;

                XRight = IntToFx16(X1) + YDiff * XDeltaRightByY;
                RRight = IntToFx16(RVtx1) + YDiff * RDeltaRightByY;
                GRight = IntToFx16(GVtx1) + YDiff * GDeltaRightByY;
                BRight = IntToFx16(BVtx1) + YDiff * BDeltaRightByY;
                ZRight = IntToFx16(ZVtx1) + YDiff * ZDeltaRightByY;
            }
            else
            {
                YStart = Y0;

                XLeft = IntToFx16(X0);
                RLeft = IntToFx16(RVtx0);
                GLeft = IntToFx16(GVtx0);
                BLeft = IntToFx16(BVtx0);
                ZLeft = IntToFx16(ZVtx0);

                XRight = IntToFx16(X1);
                RRight = IntToFx16(RVtx1);
                GRight = IntToFx16(GVtx1);
                BRight = IntToFx16(BVtx1);
                ZRight = IntToFx16(ZVtx1);
            }
        }
        else // Bottom case
        {
            // Compute deltas for coords, colors
            XDeltaLeftByY = IntToFx16(X1 - X0) / YDiff;
            RDeltaLeftByY = IntToFx16(RVtx1 - RVtx0) / YDiff;
            GDeltaLeftByY = IntToFx16(GVtx1 - GVtx0) / YDiff;
            BDeltaLeftByY = IntToFx16(BVtx1 - BVtx0) / YDiff;
            ZDeltaLeftByY = IntToFx16(ZVtx1 - ZVtx0) / YDiff;

            XDeltaRightByY = IntToFx16(X2 - X0) / YDiff;
            RDeltaRightByY = IntToFx16(RVtx2 - RVtx0) / YDiff;
            GDeltaRightByY = IntToFx16(GVtx2 - GVtx0) / YDiff;
            BDeltaRightByY = IntToFx16(BVtx2 - BVtx0) / YDiff;
            ZDeltaRightByY = IntToFx16(ZVtx2 - ZVtx0) / YDiff;

            // Clipping Y
            if (Y0 < RenderSpec.MinClip.Y)
            {
                YDiff = RenderSpec.MinClip.Y - Y0;
                YStart = RenderSpec.MinClip.Y;

                XLeft = IntToFx16(X0) + YDiff * XDeltaLeftByY;
                RLeft = IntToFx16(RVtx0) + YDiff * RDeltaLeftByY;
                GLeft = IntToFx16(GVtx0) + YDiff * GDeltaLeftByY;
                BLeft = IntToFx16(BVtx0) + YDiff * BDeltaLeftByY;
                ZLeft = IntToFx16(ZVtx0) + YDiff * ZDeltaLeftByY;

                XRight = IntToFx16(X0) + YDiff * XDeltaRightByY;
                RRight = IntToFx16(RVtx0) + YDiff * RDeltaRightByY;
                GRight = IntToFx16(GVtx0) + YDiff * GDeltaRightByY;
                BRight = IntToFx16(BVtx0) + YDiff * BDeltaRightByY;
                ZRight = IntToFx16(ZVtx0) + YDiff * ZDeltaRightByY;
            }
            else
            {
                YStart = Y0;

                XLeft = IntToFx16(X0);
                RLeft = IntToFx16(RVtx0);
                GLeft = IntToFx16(GVtx0);
                BLeft = IntToFx16(BVtx0);
                ZLeft = IntToFx16(ZVtx0);

                XRight = IntToFx16(X0);
                RRight = IntToFx16(RVtx0);
                GRight = IntToFx16(GVtx0);
                BRight = IntToFx16(BVtx0);
                ZRight = IntToFx16(ZVtx0);
            }
        }

        // Clip bottom Y
        if (Y2 > RenderSpec.MaxClip.Y)
        {
            YEnd = RenderSpec.MaxClip.Y;
        }
        else
        {
            YEnd = Y2;
        }

        // Test for clipping X
        if (X0 < RenderSpec.MinClip.X || X1 < RenderSpec.MinClip.X || X2 < RenderSpec.MinClip.X ||
            X0 > RenderSpec.MaxClip.X || X1 > RenderSpec.MaxClip.X || X2 > RenderSpec.MaxClip.X)
        {
            // Align buffer pointer
            Buffer += Pitch * YStart;
            ZBufferArray = (fx16*)ZBuffer.Buffer + (ZBuffer.Pitch * YStart);

            // Process each Y
            for (i32f Y = YStart; Y <= YEnd; ++Y)
            {
                // Compute starting values
                i32f XStart = Fx16ToIntRounded(XLeft);
                i32f XEnd = Fx16ToIntRounded(XRight);

                fx16 R = RLeft;
                fx16 G = GLeft;
                fx16 B = BLeft;

                fx16 Z = ZLeft;

                // Compute deltas for X interpolation
                i32f XDiff = XEnd - XStart;

                fx16 RDeltaByX;
                fx16 GDeltaByX;
                fx16 BDeltaByX;
                fx16 ZDeltaByX;
                if (XDiff > 0)
                {
                    RDeltaByX = (RRight - RLeft) / XDiff;
                    GDeltaByX = (GRight - GLeft) / XDiff;
                    BDeltaByX = (BRight - BLeft) / XDiff;
                    ZDeltaByX = (ZRight - ZLeft) / XDiff;
                }
                else
                {
                    RDeltaByX = (RRight - RLeft);
                    GDeltaByX = (GRight - GLeft);
                    BDeltaByX = (BRight - BLeft);
                    ZDeltaByX = (ZRight - ZLeft);
                }

                // X clipping
                if (XStart < RenderSpec.MinClip.X)
                {
                    i32 XDiff = RenderSpec.MinClip.X - XStart;
                    XStart = RenderSpec.MinClip.X;

                    R += XDiff * RDeltaByX;
                    G += XDiff * GDeltaByX;
                    B += XDiff * BDeltaByX;
                    Z += XDiff * ZDeltaByX;
                }
                if (XEnd > RenderSpec.MaxClip.X)
                {
                    XEnd = RenderSpec.MaxClip.X;
                }

                // Process each X
                for (i32f X = XStart; X <= XEnd; ++X)
                {
                    if (Z < ZBufferArray[X])
                    {
                        Buffer[X] = MAP_XRGB32(
                            Fx16ToIntRounded(R),
                            Fx16ToIntRounded(G),
                            Fx16ToIntRounded(B)
                        );

                        ZBufferArray[X] = Z;
                    }

                    // Update X values
                    R += RDeltaByX;
                    G += GDeltaByX;
                    B += BDeltaByX;
                    Z += ZDeltaByX;
                }

                // Update Y values
                XLeft += XDeltaLeftByY;
                RLeft += RDeltaLeftByY;
                GLeft += GDeltaLeftByY;
                BLeft += BDeltaLeftByY;
                ZLeft += ZDeltaLeftByY;

                XRight += XDeltaRightByY;
                RRight += RDeltaRightByY;
                GRight += GDeltaRightByY;
                BRight += BDeltaRightByY;
                ZRight += ZDeltaRightByY;

                Buffer += Pitch;
                ZBufferArray += ZBuffer.Pitch;
            }
        }
        else // Non-clipped version
        {
            // Align buffer pointer
            Buffer += Pitch * YStart;
            ZBufferArray = (fx16*)ZBuffer.Buffer + (ZBuffer.Pitch * YStart);

            // Process each Y
            for (i32f Y = YStart; Y <= YEnd; ++Y)
            {
                // Compute starting values
                i32f XStart = Fx16ToIntRounded(XLeft);
                i32f XEnd = Fx16ToIntRounded(XRight);

                fx16 R = RLeft;
                fx16 G = GLeft;
                fx16 B = BLeft;
                fx16 Z = ZLeft;

                // Compute deltas for X interpolation
                i32f XDiff = XEnd - XStart;

                fx16 RDeltaByX;
                fx16 GDeltaByX;
                fx16 BDeltaByX;
                fx16 ZDeltaByX;
                if (XDiff > 0)
                {
                    RDeltaByX = (RRight - RLeft) / XDiff;
                    GDeltaByX = (GRight - GLeft) / XDiff;
                    BDeltaByX = (BRight - BLeft) / XDiff;
                    ZDeltaByX = (ZRight - ZLeft) / XDiff;
                }
                else
                {
                    RDeltaByX = (RRight - RLeft);
                    GDeltaByX = (GRight - GLeft);
                    BDeltaByX = (BRight - BLeft);
                    ZDeltaByX = (ZRight - ZLeft);
                }

                // Process each X
                for (i32f X = XStart; X <= XEnd; ++X)
                {
                   if (Z < ZBufferArray[X])
                    {
                        Buffer[X] = MAP_XRGB32(
                            Fx16ToIntRounded(R),
                            Fx16ToIntRounded(G),
                            Fx16ToIntRounded(B)
                        );

                        ZBufferArray[X] = Z;
                    }

                    // Update X values
                    R += RDeltaByX;
                    G += GDeltaByX;
                    B += BDeltaByX;
                    Z += ZDeltaByX;
                }

                // Update Y values
                XLeft += XDeltaLeftByY;
                RLeft += RDeltaLeftByY;
                GLeft += GDeltaLeftByY;
                BLeft += BDeltaLeftByY;
                ZLeft += ZDeltaLeftByY;

                XRight += XDeltaRightByY;
                RRight += RDeltaRightByY;
                GRight += GDeltaRightByY;
                BRight += BDeltaRightByY;
                ZRight += ZDeltaRightByY;

                Buffer += Pitch;
                ZBufferArray += ZBuffer.Pitch;
            }
        }
    }
    else // General case
    {
        b32 bRestartInterpolationAtLeftHand = true;
        i32 YRestartInterpolation = Y1;

        // Clip bottom Y
        if (Y2 > RenderSpec.MaxClip.Y)
        {
            YEnd = RenderSpec.MaxClip.Y;
        }
        else
        {
            YEnd = Y2;
        }

        // Clip top Y
        if (Y1 < RenderSpec.MinClip.Y)
        {
            // Compute deltas
            i32 YDiffLeft = (Y2 - Y1);
            XDeltaLeftByY = IntToFx16(X2 - X1) / YDiffLeft;
            RDeltaLeftByY = IntToFx16(RVtx2 - RVtx1) / YDiffLeft;
            GDeltaLeftByY = IntToFx16(GVtx2 - GVtx1) / YDiffLeft;
            BDeltaLeftByY = IntToFx16(BVtx2 - BVtx1) / YDiffLeft;
            ZDeltaLeftByY = IntToFx16(ZVtx2 - ZVtx1) / YDiffLeft;

            i32 YDiffRight = (Y2 - Y0);
            XDeltaRightByY = IntToFx16(X2 - X0) / YDiffRight;
            RDeltaRightByY = IntToFx16(RVtx2 - RVtx0) / YDiffRight;
            GDeltaRightByY = IntToFx16(GVtx2 - GVtx0) / YDiffRight;
            BDeltaRightByY = IntToFx16(BVtx2 - BVtx0) / YDiffRight;
            ZDeltaRightByY = IntToFx16(ZVtx2 - ZVtx0) / YDiffRight;

            // Do clipping
            YDiffLeft = (RenderSpec.MinClip.Y - Y1);
            XLeft = IntToFx16(X1) + YDiffLeft * XDeltaLeftByY;
            RLeft = IntToFx16(RVtx1) + YDiffLeft * RDeltaLeftByY;
            GLeft = IntToFx16(GVtx1) + YDiffLeft * GDeltaLeftByY;
            BLeft = IntToFx16(BVtx1) + YDiffLeft * BDeltaLeftByY;
            ZLeft = IntToFx16(ZVtx1) + YDiffLeft * ZDeltaLeftByY;

            YDiffRight = (RenderSpec.MinClip.Y - Y0);
            XRight = IntToFx16(X0) + YDiffRight * XDeltaRightByY;
            RRight = IntToFx16(RVtx0) + YDiffRight * RDeltaRightByY;
            GRight = IntToFx16(GVtx0) + YDiffRight * GDeltaRightByY;
            BRight = IntToFx16(BVtx0) + YDiffRight * BDeltaRightByY;
            ZRight = IntToFx16(ZVtx0) + YDiffRight * ZDeltaRightByY;

            YStart = RenderSpec.MinClip.Y;

            /* NOTE(sean):
                Test if we need swap to keep rendering left to right.
                It can happen because we assume that
                Y1 is on left hand side and Y2 on right.
             */
            if (XDeltaRightByY > XDeltaLeftByY)
            {
                VL_SWAP(XDeltaLeftByY, XDeltaRightByY, TempInt);
                VL_SWAP(RDeltaLeftByY, RDeltaRightByY, TempInt);
                VL_SWAP(GDeltaLeftByY, GDeltaRightByY, TempInt);
                VL_SWAP(BDeltaLeftByY, BDeltaRightByY, TempInt);
                VL_SWAP(ZDeltaLeftByY, ZDeltaRightByY, TempInt);

                VL_SWAP(XLeft, XRight, TempInt);
                VL_SWAP(RLeft, RRight, TempInt);
                VL_SWAP(GLeft, GRight, TempInt);
                VL_SWAP(BLeft, BRight, TempInt);
                VL_SWAP(ZLeft, ZRight, TempInt);

                VL_SWAP(X1, X2, TempInt);
                VL_SWAP(Y1, Y2, TempInt);
                VL_SWAP(RVtx1, RVtx2, TempInt);
                VL_SWAP(GVtx1, GVtx2, TempInt);
                VL_SWAP(BVtx1, BVtx2, TempInt);
                VL_SWAP(ZVtx1, ZVtx2, TempInt);

                bRestartInterpolationAtLeftHand = false; // Restart at right hand side
            }
        }
        else if (Y0 < RenderSpec.MinClip.Y)
        {
            i32 YDiffLeft = (Y1 - Y0);
            XDeltaLeftByY = IntToFx16(X1 - X0) / YDiffLeft;
            RDeltaLeftByY = IntToFx16(RVtx1 - RVtx0) / YDiffLeft;
            GDeltaLeftByY = IntToFx16(GVtx1 - GVtx0) / YDiffLeft;
            BDeltaLeftByY = IntToFx16(BVtx1 - BVtx0) / YDiffLeft;
            ZDeltaLeftByY = IntToFx16(ZVtx1 - ZVtx0) / YDiffLeft;

            i32 YDiffRight = (Y2 - Y0);
            XDeltaRightByY = IntToFx16(X2 - X0) / YDiffRight;
            RDeltaRightByY = IntToFx16(RVtx2 - RVtx0) / YDiffRight;
            GDeltaRightByY = IntToFx16(GVtx2 - GVtx0) / YDiffRight;
            BDeltaRightByY = IntToFx16(BVtx2 - BVtx0) / YDiffRight;
            ZDeltaRightByY = IntToFx16(ZVtx2 - ZVtx0) / YDiffRight;

            i32 YDiff = (RenderSpec.MinClip.Y - Y0);
            XLeft = IntToFx16(X0) + YDiff * XDeltaLeftByY;
            RLeft = IntToFx16(RVtx0) + YDiff * RDeltaLeftByY;
            GLeft = IntToFx16(GVtx0) + YDiff * GDeltaLeftByY;
            BLeft = IntToFx16(BVtx0) + YDiff * BDeltaLeftByY;
            ZLeft = IntToFx16(ZVtx0) + YDiff * ZDeltaLeftByY;

            XRight = IntToFx16(X0) + YDiff * XDeltaRightByY;
            RRight = IntToFx16(RVtx0) + YDiff * RDeltaRightByY;
            GRight = IntToFx16(GVtx0) + YDiff * GDeltaRightByY;
            BRight = IntToFx16(BVtx0) + YDiff * BDeltaRightByY;
            ZRight = IntToFx16(ZVtx0) + YDiff * ZDeltaRightByY;

            YStart = RenderSpec.MinClip.Y;

            /* NOTE(sean):
                Test if we need swap to keep rendering left to right.
                It can happen because we assume that
                Y1 is on left hand side and Y2 on right.
             */
            // TODO(sean): Test if we can simplify it
            if (XDeltaRightByY < XDeltaLeftByY)
            {
                VL_SWAP(XDeltaLeftByY, XDeltaRightByY, TempInt);
                VL_SWAP(RDeltaLeftByY, RDeltaRightByY, TempInt);
                VL_SWAP(GDeltaLeftByY, GDeltaRightByY, TempInt);
                VL_SWAP(BDeltaLeftByY, BDeltaRightByY, TempInt);
                VL_SWAP(ZDeltaLeftByY, ZDeltaRightByY, TempInt);

                VL_SWAP(XLeft, XRight, TempInt);
                VL_SWAP(RLeft, RRight, TempInt);
                VL_SWAP(GLeft, GRight, TempInt);
                VL_SWAP(BLeft, BRight, TempInt);
                VL_SWAP(ZLeft, ZRight, TempInt);

                VL_SWAP(X1, X2, TempInt);
                VL_SWAP(Y1, Y2, TempInt);
                VL_SWAP(RVtx1, RVtx2, TempInt);
                VL_SWAP(GVtx1, GVtx2, TempInt);
                VL_SWAP(BVtx1, BVtx2, TempInt);
                VL_SWAP(ZVtx1, ZVtx2, TempInt);

                bRestartInterpolationAtLeftHand = false; // Restart at right hand side
            }
        }
        else // No top Y clipping
        {
            i32 YDiffLeft = (Y1 - Y0);
            XDeltaLeftByY = IntToFx16(X1 - X0) / YDiffLeft;
            RDeltaLeftByY = IntToFx16(RVtx1 - RVtx0) / YDiffLeft;
            GDeltaLeftByY = IntToFx16(GVtx1 - GVtx0) / YDiffLeft;
            BDeltaLeftByY = IntToFx16(BVtx1 - BVtx0) / YDiffLeft;
            ZDeltaLeftByY = IntToFx16(ZVtx1 - ZVtx0) / YDiffLeft;

            i32 YDiffRight = (Y2 - Y0);
            XDeltaRightByY = IntToFx16(X2 - X0) / YDiffRight;
            RDeltaRightByY = IntToFx16(RVtx2 - RVtx0) / YDiffRight;
            GDeltaRightByY = IntToFx16(GVtx2 - GVtx0) / YDiffRight;
            BDeltaRightByY = IntToFx16(BVtx2 - BVtx0) / YDiffRight;
            ZDeltaRightByY = IntToFx16(ZVtx2 - ZVtx0) / YDiffRight;

            XRight = XLeft = IntToFx16(X0);
            RRight = RLeft = IntToFx16(RVtx0);
            GRight = GLeft = IntToFx16(GVtx0);
            BRight = BLeft = IntToFx16(BVtx0);
            ZRight = ZLeft = IntToFx16(ZVtx0);

            YStart = Y0;

            /* NOTE(sean):
                Test if we need swap to keep rendering left to right.
                It can happen because we assume that
                Y1 is on left hand side and Y2 on right.
             */
            // TODO(sean): Test if we can simplify it
            if (XDeltaRightByY < XDeltaLeftByY)
            {
                VL_SWAP(XDeltaLeftByY, XDeltaRightByY, TempInt);
                VL_SWAP(RDeltaLeftByY, RDeltaRightByY, TempInt);
                VL_SWAP(GDeltaLeftByY, GDeltaRightByY, TempInt);
                VL_SWAP(BDeltaLeftByY, BDeltaRightByY, TempInt);
                VL_SWAP(ZDeltaLeftByY, ZDeltaRightByY, TempInt);

                VL_SWAP(XLeft, XRight, TempInt);
                VL_SWAP(RLeft, RRight, TempInt);
                VL_SWAP(GLeft, GRight, TempInt);
                VL_SWAP(BLeft, BRight, TempInt);
                VL_SWAP(ZLeft, ZRight, TempInt);

                VL_SWAP(X1, X2, TempInt);
                VL_SWAP(Y1, Y2, TempInt);
                VL_SWAP(RVtx1, RVtx2, TempInt);
                VL_SWAP(GVtx1, GVtx2, TempInt);
                VL_SWAP(BVtx1, BVtx2, TempInt);
                VL_SWAP(ZVtx1, ZVtx2, TempInt);

                bRestartInterpolationAtLeftHand = false; // Restart at right hand side
            }
        }

        // Test for clipping X
        if (X0 < RenderSpec.MinClip.X || X1 < RenderSpec.MinClip.X || X2 < RenderSpec.MinClip.X ||
            X0 > RenderSpec.MaxClip.X || X1 > RenderSpec.MaxClip.X || X2 > RenderSpec.MaxClip.X)
        {
            // Align video buffer
            Buffer += Pitch * YStart;
            ZBufferArray = (fx16*)ZBuffer.Buffer + (ZBuffer.Pitch * YStart);

            // Process each Y
            for (i32f Y = YStart; Y <= YEnd; ++Y)
            {
                // Compute initial values
                i32 XStart = Fx16ToIntRounded(XLeft);
                i32 XEnd   = Fx16ToIntRounded(XRight);

                fx16 R = RLeft;
                fx16 G = GLeft;
                fx16 B = BLeft;
                fx16 Z = ZLeft;

                // Compute interpolants
                fx16 RDeltaByX;
                fx16 GDeltaByX;
                fx16 BDeltaByX;
                fx16 ZDeltaByX;

                i32 XDiff = XEnd - XStart;
                if (XDiff > 0)
                {
                    RDeltaByX = (RRight - RLeft) / XDiff;
                    GDeltaByX = (GRight - GLeft) / XDiff;
                    BDeltaByX = (BRight - BLeft) / XDiff;
                    ZDeltaByX = (ZRight - ZLeft) / XDiff;
                }
                else
                {
                    RDeltaByX = (RRight - RLeft);
                    GDeltaByX = (GRight - GLeft);
                    BDeltaByX = (BRight - BLeft);
                    ZDeltaByX = (ZRight - ZLeft);
                }

                // Test if we need clipping
                if (XStart < RenderSpec.MinClip.X)
                {
                    XDiff = RenderSpec.MinClip.X - XStart;
                    XStart = RenderSpec.MinClip.X;

                    R += RDeltaByX * XDiff;
                    G += GDeltaByX * XDiff;
                    B += BDeltaByX * XDiff;
                    Z += ZDeltaByX * XDiff;
                }
                if (XEnd > RenderSpec.MaxClip.X)
                {
                    XEnd = RenderSpec.MaxClip.X;
                }

                // Process each X
                for (i32f X = XStart; X <= XEnd; ++X)
                {
                   if (Z < ZBufferArray[X])
                    {
                        Buffer[X] = MAP_XRGB32(
                            Fx16ToIntRounded(R),
                            Fx16ToIntRounded(G),
                            Fx16ToIntRounded(B)
                        );

                        ZBufferArray[X] = Z;
                    }

                    R += RDeltaByX;
                    G += GDeltaByX;
                    B += BDeltaByX;
                    Z += ZDeltaByX;
                }

                // Update values those change along Y
                XLeft += XDeltaLeftByY;
                RLeft += RDeltaLeftByY;
                GLeft += GDeltaLeftByY;
                BLeft += BDeltaLeftByY;
                ZLeft += ZDeltaLeftByY;

                XRight += XDeltaRightByY;
                RRight += RDeltaRightByY;
                GRight += GDeltaRightByY;
                BRight += BDeltaRightByY;
                ZRight += ZDeltaRightByY;

                Buffer += Pitch;
                ZBufferArray += ZBuffer.Pitch;

                // Test for changing interpolant
                if (Y == YRestartInterpolation)
                {
                    if (bRestartInterpolationAtLeftHand)
                    {
                        // Compute new values to get from Y1 to Y2
                        i32 YDiff = (Y2 - Y1);

                        XDeltaLeftByY = IntToFx16(X2 - X1) / YDiff;
                        RDeltaLeftByY = IntToFx16(RVtx2 - RVtx1) / YDiff;
                        GDeltaLeftByY = IntToFx16(GVtx2 - GVtx1) / YDiff;
                        BDeltaLeftByY = IntToFx16(BVtx2 - BVtx1) / YDiff;
                        ZDeltaLeftByY = IntToFx16(ZVtx2 - ZVtx1) / YDiff;

                        XLeft = IntToFx16(X1);
                        RLeft = IntToFx16(RVtx1);
                        GLeft = IntToFx16(GVtx1);
                        BLeft = IntToFx16(BVtx1);
                        ZLeft = IntToFx16(ZVtx1);

                        // Align down on 1 Y
                        XLeft += XDeltaLeftByY;
                        RLeft += RDeltaLeftByY;
                        GLeft += GDeltaLeftByY;
                        BLeft += BDeltaLeftByY;
                        ZLeft += ZDeltaLeftByY;
                    }
                    else
                    {
                        // Compute new values to get from Y2 to Y1 because we swapped them
                        i32 YDiff = (Y1 - Y2);

                        XDeltaRightByY = IntToFx16(X1 - X2) / YDiff;
                        RDeltaRightByY = IntToFx16(RVtx1 - RVtx2) / YDiff;
                        GDeltaRightByY = IntToFx16(GVtx1 - GVtx2) / YDiff;
                        BDeltaRightByY = IntToFx16(BVtx1 - BVtx2) / YDiff;
                        ZDeltaRightByY = IntToFx16(ZVtx1 - ZVtx2) / YDiff;

                        XRight = IntToFx16(X2);
                        RRight = IntToFx16(RVtx2);
                        GRight = IntToFx16(GVtx2);
                        BRight = IntToFx16(BVtx2);
                        ZRight = IntToFx16(ZVtx2);

                        // Align down on 1 Y
                        XRight += XDeltaRightByY;
                        RRight += RDeltaRightByY;
                        GRight += GDeltaRightByY;
                        BRight += BDeltaRightByY;
                        ZRight += ZDeltaRightByY;
                    }
                }
            }
        }
        else // No X clipping
        {
            // Align video buffer
            Buffer += Pitch * YStart;
            ZBufferArray = (fx16*)ZBuffer.Buffer + (ZBuffer.Pitch * YStart);

            // Process each Y
            for (i32f Y = YStart; Y <= YEnd; ++Y)
            {
                // Compute initial values
                i32 XStart = Fx16ToIntRounded(XLeft);
                i32 XEnd   = Fx16ToIntRounded(XRight);

                fx16 R = RLeft;
                fx16 G = GLeft;
                fx16 B = BLeft;
                fx16 Z = ZLeft;

                // Compute interpolants
                fx16 RDeltaByX;
                fx16 GDeltaByX;
                fx16 BDeltaByX;
                fx16 ZDeltaByX;

                i32 XDiff = XEnd - XStart;
                if (XDiff > 0)
                {
                    RDeltaByX = (RRight - RLeft) / XDiff;
                    GDeltaByX = (GRight - GLeft) / XDiff;
                    BDeltaByX = (BRight - BLeft) / XDiff;
                    ZDeltaByX = (ZRight - ZLeft) / XDiff;
                }
                else
                {
                    RDeltaByX = (RRight - RLeft);
                    GDeltaByX = (GRight - GLeft);
                    BDeltaByX = (BRight - BLeft);
                    ZDeltaByX = (ZRight - ZLeft);
                }

                // Process each X
                for (i32f X = XStart; X <= XEnd; ++X)
                {
                   if (Z < ZBufferArray[X])
                    {
                        Buffer[X] = MAP_XRGB32(
                            Fx16ToIntRounded(R),
                            Fx16ToIntRounded(G),
                            Fx16ToIntRounded(B)
                        );

                        ZBufferArray[X] = Z;
                    }

                    R += RDeltaByX;
                    G += GDeltaByX;
                    B += BDeltaByX;
                    Z += ZDeltaByX;
                }

                // Update values those change along Y
                XLeft += XDeltaLeftByY;
                RLeft += RDeltaLeftByY;
                GLeft += GDeltaLeftByY;
                BLeft += BDeltaLeftByY;
                ZLeft += ZDeltaLeftByY;

                XRight += XDeltaRightByY;
                RRight += RDeltaRightByY;
                GRight += GDeltaRightByY;
                BRight += BDeltaRightByY;
                ZRight += ZDeltaRightByY;

                Buffer += Pitch;
                ZBufferArray += ZBuffer.Pitch;

                // Test for changing interpolant
                if (Y == YRestartInterpolation)
                {
                    if (bRestartInterpolationAtLeftHand)
                    {
                        // Compute new values to get from Y1 to Y2
                        i32 YDiff = (Y2 - Y1);

                        XDeltaLeftByY = IntToFx16(X2 - X1) / YDiff;
                        RDeltaLeftByY = IntToFx16(RVtx2 - RVtx1) / YDiff;
                        GDeltaLeftByY = IntToFx16(GVtx2 - GVtx1) / YDiff;
                        BDeltaLeftByY = IntToFx16(BVtx2 - BVtx1) / YDiff;
                        ZDeltaLeftByY = IntToFx16(ZVtx2 - ZVtx1) / YDiff;

                        XLeft = IntToFx16(X1);
                        RLeft = IntToFx16(RVtx1);
                        GLeft = IntToFx16(GVtx1);
                        BLeft = IntToFx16(BVtx1);
                        ZLeft = IntToFx16(ZVtx1);

                        // Align down on 1 Y
                        XLeft += XDeltaLeftByY;
                        RLeft += RDeltaLeftByY;
                        GLeft += GDeltaLeftByY;
                        BLeft += BDeltaLeftByY;
                        ZLeft += ZDeltaLeftByY;
                    }
                    else
                    {
                        // Compute new values to get from Y2 to Y1 because we swapped them
                        i32 YDiff = (Y1 - Y2);

                        XDeltaRightByY = IntToFx16(X1 - X2) / YDiff;
                        RDeltaRightByY = IntToFx16(RVtx1 - RVtx2) / YDiff;
                        GDeltaRightByY = IntToFx16(GVtx1 - GVtx2) / YDiff;
                        BDeltaRightByY = IntToFx16(BVtx1 - BVtx2) / YDiff;
                        ZDeltaRightByY = IntToFx16(ZVtx1 - ZVtx2) / YDiff;

                        XRight = IntToFx16(X2);
                        RRight = IntToFx16(RVtx2);
                        GRight = IntToFx16(GVtx2);
                        BRight = IntToFx16(BVtx2);
                        ZRight = IntToFx16(ZVtx2);

                        // Align down on 1 Y
                        XRight += XDeltaRightByY;
                        RRight += RDeltaRightByY;
                        GRight += GDeltaRightByY;
                        BRight += BDeltaRightByY;
                        ZRight += ZDeltaRightByY;
                    }
                }
            }
        }
    }
}

void VRenderer::DrawTexturedTriangle(u32* Buffer, i32 Pitch, const VPolyFace& Poly) const
{
    enum class ETriangleCase
    {
        Top,
        Bottom,
        General
    };

    i32 V0 = 0, V1 = 1, V2 = 2;

    // Sort by Y
    i32 TempInt;
    if (Poly.TransVtx[V1].Y < Poly.TransVtx[V0].Y)
    {
        VL_SWAP(V0, V1, TempInt);
    }
    if (Poly.TransVtx[V2].Y < Poly.TransVtx[V0].Y)
    {
        VL_SWAP(V0, V2, TempInt);
    }
    if (Poly.TransVtx[V2].Y < Poly.TransVtx[V1].Y)
    {
        VL_SWAP(V1, V2, TempInt);
    }

    // Test if we can't see it
    if (Poly.TransVtx[V2].Y < RenderSpec.MinClipFloat.Y ||
        Poly.TransVtx[V0].Y > RenderSpec.MaxClipFloat.Y ||
        (Poly.TransVtx[V0].X < RenderSpec.MinClipFloat.X && Poly.TransVtx[V1].X < RenderSpec.MinClipFloat.X && Poly.TransVtx[V2].X < RenderSpec.MinClipFloat.X) ||
        (Poly.TransVtx[V0].X > RenderSpec.MaxClipFloat.X && Poly.TransVtx[V1].X > RenderSpec.MaxClipFloat.X && Poly.TransVtx[V2].X > RenderSpec.MaxClipFloat.X))
    {
        return;
    }

    // Convert Y to integers
    i32 Y0 = (i32)(Poly.TransVtx[V0].Y + 0.5f);
    i32 Y1 = (i32)(Poly.TransVtx[V1].Y + 0.5f);
    i32 Y2 = (i32)(Poly.TransVtx[V2].Y + 0.5f);

    // Found triangle case and sort vertices by X
    ETriangleCase TriangleCase;
    if (Y0 == Y1)
    {
        if (Poly.TransVtx[V1].X < Poly.TransVtx[V0].X)
        {
            VL_SWAP(V0, V1, TempInt);
            VL_SWAP(Y0, Y1, TempInt);
        }
        TriangleCase = ETriangleCase::Top;
    }
    else if (Y1 == Y2)
    {
        if (Poly.TransVtx[V2].X < Poly.TransVtx[V1].X)
        {
            VL_SWAP(V1, V2, TempInt);
            VL_SWAP(Y1, Y2, TempInt);
        }
        TriangleCase = ETriangleCase::Bottom;
    }
    else
    {
        TriangleCase = ETriangleCase::General;
    }

    // Convert coords to integer
    i32 X0 = (i32)(Poly.TransVtx[V0].X + 0.5f);
    i32 X1 = (i32)(Poly.TransVtx[V1].X + 0.5f);
    i32 X2 = (i32)(Poly.TransVtx[V2].X + 0.5f);

    // Vertical, horizontal triangle test
    if ((Y0 == Y1 && Y1 == Y2) || (X0 == X1 && X1 == X2))
    {
        return;
    }

    i32 YStart;
    i32 YEnd;

    i32 UVtx0 = (i32)Poly.TransVtx[V0].U, VVtx0 = (i32)Poly.TransVtx[V0].V;
    i32 UVtx1 = (i32)Poly.TransVtx[V1].U, VVtx1 = (i32)Poly.TransVtx[V1].V;
    i32 UVtx2 = (i32)Poly.TransVtx[V2].U, VVtx2 = (i32)Poly.TransVtx[V2].V;

    i32 ZVtx0 = (i32)(Poly.TransVtx[V0].Z + 0.5f);
    i32 ZVtx1 = (i32)(Poly.TransVtx[V1].Z + 0.5f);
    i32 ZVtx2 = (i32)(Poly.TransVtx[V2].Z + 0.5f);

    // Fixed coords, color channels for rasterization
    fx16 XLeft, VLeft, ULeft;
    fx16 XRight, VRight, URight;
    fx16 ZLeft, ZRight;

    // Coords, colors fixed deltas by Y
    fx16 XDeltaLeftByY;
    fx16 UDeltaLeftByY, VDeltaLeftByY;
    fx16 ZDeltaLeftByY;

    fx16 XDeltaRightByY;
    fx16 UDeltaRightByY, VDeltaRightByY;
    fx16 ZDeltaRightByY;

    // Extract base color
    u32 RBase = Poly.LitColor[0].R;
    u32 GBase = Poly.LitColor[0].G;
    u32 BBase = Poly.LitColor[0].B;

    // Extract texture
    u32* TextureBuffer;
    i32 TexturePitch;
    Poly.Texture->Lock(TextureBuffer, TexturePitch);

    // Extract z-buffer
    fx16* ZBufferArray;

    if (TriangleCase == ETriangleCase::Top ||
        TriangleCase == ETriangleCase::Bottom)
    {
        i32 YDiff = Y2 - Y0;

        if (TriangleCase == ETriangleCase::Top)
        {
            // Compute deltas for coords, colors
            XDeltaLeftByY = IntToFx16(X2 - X0) / YDiff;
            UDeltaLeftByY = IntToFx16(UVtx2 - UVtx0) / YDiff;
            VDeltaLeftByY = IntToFx16(VVtx2 - VVtx0) / YDiff;
            ZDeltaLeftByY = IntToFx16(ZVtx2 - ZVtx0) / YDiff;

            XDeltaRightByY = IntToFx16(X2 - X1) / YDiff;
            UDeltaRightByY = IntToFx16(UVtx2 - UVtx1) / YDiff;
            VDeltaRightByY = IntToFx16(VVtx2 - VVtx1) / YDiff;
            ZDeltaRightByY = IntToFx16(ZVtx2 - ZVtx1) / YDiff;

            // Clipping Y
            if (Y0 < RenderSpec.MinClip.Y)
            {
                YDiff = RenderSpec.MinClip.Y - Y0;
                YStart = RenderSpec.MinClip.Y;

                XLeft = IntToFx16(X0) + YDiff * XDeltaLeftByY;
                ULeft = IntToFx16(UVtx0) + YDiff * UDeltaLeftByY;
                VLeft = IntToFx16(VVtx0) + YDiff * VDeltaLeftByY;
                ZLeft = IntToFx16(ZVtx0) + YDiff * ZDeltaLeftByY;

                XRight = IntToFx16(X1) + YDiff * XDeltaRightByY;
                URight = IntToFx16(UVtx1) + YDiff * UDeltaRightByY;
                VRight = IntToFx16(VVtx1) + YDiff * VDeltaRightByY;
                ZRight = IntToFx16(ZVtx1) + YDiff * ZDeltaRightByY;
            }
            else
            {
                YStart = Y0;

                XLeft = IntToFx16(X0);
                ULeft = IntToFx16(UVtx0);
                VLeft = IntToFx16(VVtx0);
                ZLeft = IntToFx16(ZVtx0);

                XRight = IntToFx16(X1);
                URight = IntToFx16(UVtx1);
                VRight = IntToFx16(VVtx1);
                ZRight = IntToFx16(ZVtx1);
            }
        }
        else // Bottom case
        {
            // Compute deltas for coords, colors
            XDeltaLeftByY = IntToFx16(X1 - X0) / YDiff;
            UDeltaLeftByY = IntToFx16(UVtx1 - UVtx0) / YDiff;
            VDeltaLeftByY = IntToFx16(VVtx1 - VVtx0) / YDiff;
            ZDeltaLeftByY = IntToFx16(ZVtx1 - ZVtx0) / YDiff;

            XDeltaRightByY = IntToFx16(X2 - X0) / YDiff;
            UDeltaRightByY = IntToFx16(UVtx2 - UVtx0) / YDiff;
            VDeltaRightByY = IntToFx16(VVtx2 - VVtx0) / YDiff;
            ZDeltaRightByY = IntToFx16(ZVtx2 - ZVtx0) / YDiff;

            // Clipping Y
            if (Y0 < RenderSpec.MinClip.Y)
            {
                YDiff = RenderSpec.MinClip.Y - Y0;
                YStart = RenderSpec.MinClip.Y;

                XLeft = IntToFx16(X0) + YDiff * XDeltaLeftByY;
                ULeft = IntToFx16(UVtx0) + YDiff * UDeltaLeftByY;
                VLeft = IntToFx16(VVtx0) + YDiff * VDeltaLeftByY;
                ZLeft = IntToFx16(ZVtx0) + YDiff * ZDeltaLeftByY;

                XRight = IntToFx16(X0) + YDiff * XDeltaRightByY;
                URight = IntToFx16(UVtx0) + YDiff * UDeltaRightByY;
                VRight = IntToFx16(VVtx0) + YDiff * VDeltaRightByY;
                ZRight = IntToFx16(ZVtx0) + YDiff * ZDeltaRightByY;
            }
            else
            {
                YStart = Y0;

                XLeft = IntToFx16(X0);
                ULeft = IntToFx16(UVtx0);
                VLeft = IntToFx16(VVtx0);
                ZLeft = IntToFx16(ZVtx0);

                XRight = IntToFx16(X0);
                URight = IntToFx16(UVtx0);
                VRight = IntToFx16(VVtx0);
                ZRight = IntToFx16(ZVtx0);
            }
        }

        // Clip bottom Y
        if (Y2 > RenderSpec.MaxClip.Y)
        {
            YEnd = RenderSpec.MaxClip.Y;
        }
        else
        {
            YEnd = Y2;
        }

        // Test for clipping X
        if (X0 < RenderSpec.MinClip.X || X1 < RenderSpec.MinClip.X || X2 < RenderSpec.MinClip.X ||
            X0 > RenderSpec.MaxClip.X || X1 > RenderSpec.MaxClip.X || X2 > RenderSpec.MaxClip.X)
        {
            // Align buffer pointer
            Buffer += Pitch * YStart;
            ZBufferArray = (fx16*)ZBuffer.Buffer + (ZBuffer.Pitch * YStart);

            // Process each Y
            for (i32f Y = YStart; Y <= YEnd; ++Y)
            {
                // Compute starting values
                i32f XStart = Fx16ToIntRounded(XLeft);
                i32f XEnd = Fx16ToIntRounded(XRight);

                fx16 U = ULeft;
                fx16 V = VLeft;
                fx16 Z = ZLeft;

                // Compute deltas for X interpolation
                i32f XDiff = XEnd - XStart;

                fx16 UDeltaByX;
                fx16 VDeltaByX;
                fx16 ZDeltaByX;
                if (XDiff > 0)
                {
                    UDeltaByX = (URight - ULeft) / XDiff;
                    VDeltaByX = (VRight - VLeft) / XDiff;
                    ZDeltaByX = (ZRight - ZLeft) / XDiff;
                }
                else
                {
                    UDeltaByX = (URight - ULeft);
                    VDeltaByX = (VRight - VLeft);
                    ZDeltaByX = (ZRight - ZLeft);
                }

                // X clipping
                if (XStart < RenderSpec.MinClip.X)
                {
                    i32 XDiff = RenderSpec.MinClip.X - XStart;
                    XStart = RenderSpec.MinClip.X;

                    U += XDiff * UDeltaByX;
                    V += XDiff * VDeltaByX;
                    Z += XDiff * ZDeltaByX;
                }
                if (XEnd > RenderSpec.MaxClip.X)
                {
                    XEnd = RenderSpec.MaxClip.X;
                }

                // Process each X
                for (i32f X = XStart; X <= XEnd; ++X)
                {
                    if (Z < ZBufferArray[X])
                    {
                        VColorARGB Pixel = TextureBuffer[Fx16ToIntRounded(V) * TexturePitch + Fx16ToIntRounded(U)];
                        Pixel.R = (Pixel.R * RBase) >> 8;
                        Pixel.G = (Pixel.G * GBase) >> 8;
                        Pixel.B = (Pixel.B * BBase) >> 8;
                        Buffer[X] = (u32)Pixel;

                        ZBufferArray[X] = Z;
                    }

                    // Update X values
                    U += UDeltaByX;
                    V += VDeltaByX;
                    Z += ZDeltaByX;
                }

                // Update Y values
                XLeft += XDeltaLeftByY;
                ULeft += UDeltaLeftByY;
                VLeft += VDeltaLeftByY;
                ZLeft += ZDeltaLeftByY;

                XRight += XDeltaRightByY;
                URight += UDeltaRightByY;
                VRight += VDeltaRightByY;
                ZRight += ZDeltaRightByY;

                Buffer += Pitch;
                ZBufferArray += ZBuffer.Pitch;
            }
        }
        else // Non-clipped version
        {
            // Align buffer pointer
            Buffer += Pitch * YStart;
            ZBufferArray = (fx16*)ZBuffer.Buffer + (ZBuffer.Pitch * YStart);

            // Process each Y
            for (i32f Y = YStart; Y <= YEnd; ++Y)
            {
                // Compute starting values
                i32f XStart = Fx16ToIntRounded(XLeft);
                i32f XEnd = Fx16ToIntRounded(XRight);

                fx16 U = ULeft;
                fx16 V = VLeft;
                fx16 Z = ZLeft;

                // Compute deltas for X interpolation
                i32f XDiff = XEnd - XStart;

                fx16 UDeltaByX;
                fx16 VDeltaByX;
                fx16 ZDeltaByX;
                if (XDiff > 0)
                {
                    UDeltaByX = (URight - ULeft) / XDiff;
                    VDeltaByX = (VRight - VLeft) / XDiff;
                    ZDeltaByX = (ZRight - ZLeft) / XDiff;
                }
                else
                {
                    UDeltaByX = (URight - ULeft);
                    VDeltaByX = (VRight - VLeft);
                    ZDeltaByX = (ZRight - ZLeft);
                }

                // Process each X
                for (i32f X = XStart; X <= XEnd; ++X)
                {
                    if (Z < ZBufferArray[X])
                    {
                        VColorARGB Pixel = TextureBuffer[Fx16ToIntRounded(V) * TexturePitch + Fx16ToIntRounded(U)];
                        Pixel.R = (Pixel.R * RBase) >> 8;
                        Pixel.G = (Pixel.G * GBase) >> 8;
                        Pixel.B = (Pixel.B * BBase) >> 8;
                        Buffer[X] = (u32)Pixel;

                        ZBufferArray[X] = Z;
                    }

                    // Update X values
                    U += UDeltaByX;
                    V += VDeltaByX;
                    Z += ZDeltaByX;
                }

                // Update Y values
                XLeft += XDeltaLeftByY;
                ULeft += UDeltaLeftByY;
                VLeft += VDeltaLeftByY;
                ZLeft += ZDeltaLeftByY;

                XRight += XDeltaRightByY;
                URight += UDeltaRightByY;
                VRight += VDeltaRightByY;
                ZRight += ZDeltaRightByY;

                Buffer += Pitch;
                ZBufferArray += ZBuffer.Pitch;
            }
        }
    }
    else // General case
    {
        b32 bRestartInterpolationAtLeftHand = true;
        i32 YRestartInterpolation = Y1;

        // Clip bottom Y
        if (Y2 > RenderSpec.MaxClip.Y)
        {
            YEnd = RenderSpec.MaxClip.Y;
        }
        else
        {
            YEnd = Y2;
        }

        // Clip top Y
        if (Y1 < RenderSpec.MinClip.Y)
        {
            // Compute deltas
            i32 YDiffLeft = (Y2 - Y1);
            XDeltaLeftByY = IntToFx16(X2 - X1) / YDiffLeft;
            UDeltaLeftByY = IntToFx16(UVtx2 - UVtx1) / YDiffLeft;
            VDeltaLeftByY = IntToFx16(VVtx2 - VVtx1) / YDiffLeft;
            ZDeltaLeftByY = IntToFx16(ZVtx2 - ZVtx1) / YDiffLeft;

            i32 YDiffRight = (Y2 - Y0);
            XDeltaRightByY = IntToFx16(X2 - X0) / YDiffRight;
            UDeltaRightByY = IntToFx16(UVtx2 - UVtx0) / YDiffRight;
            VDeltaRightByY = IntToFx16(VVtx2 - VVtx0) / YDiffRight;
            ZDeltaRightByY = IntToFx16(ZVtx2 - ZVtx0) / YDiffRight;

            // Do clipping
            YDiffLeft = (RenderSpec.MinClip.Y - Y1);
            XLeft = IntToFx16(X1) + YDiffLeft * XDeltaLeftByY;
            ULeft = IntToFx16(UVtx1) + YDiffLeft * UDeltaLeftByY;
            VLeft = IntToFx16(VVtx1) + YDiffLeft * VDeltaLeftByY;
            ZLeft = IntToFx16(ZVtx1) + YDiffLeft * ZDeltaLeftByY;

            YDiffRight = (RenderSpec.MinClip.Y - Y0);
            XRight = IntToFx16(X0) + YDiffRight * XDeltaRightByY;
            URight = IntToFx16(UVtx0) + YDiffRight * UDeltaRightByY;
            VRight = IntToFx16(VVtx0) + YDiffRight * VDeltaRightByY;
            ZRight = IntToFx16(ZVtx0) + YDiffRight * ZDeltaRightByY;

            YStart = RenderSpec.MinClip.Y;

            /* NOTE(sean):
                Test if we need swap to keep rendering left to right.
                It can happen because we assume that
                Y1 is on left hand side and Y2 on right.
             */
            if (XDeltaRightByY > XDeltaLeftByY)
            {
                VL_SWAP(XDeltaLeftByY, XDeltaRightByY, TempInt);
                VL_SWAP(UDeltaLeftByY, UDeltaRightByY, TempInt);
                VL_SWAP(VDeltaLeftByY, VDeltaRightByY, TempInt);
                VL_SWAP(ZDeltaLeftByY, ZDeltaRightByY, TempInt);

                VL_SWAP(XLeft, XRight, TempInt);
                VL_SWAP(ULeft, URight, TempInt);
                VL_SWAP(VLeft, VRight, TempInt);
                VL_SWAP(ZLeft, ZRight, TempInt);

                VL_SWAP(X1, X2, TempInt);
                VL_SWAP(Y1, Y2, TempInt);
                VL_SWAP(UVtx1, UVtx2, TempInt);
                VL_SWAP(VVtx1, VVtx2, TempInt);
                VL_SWAP(ZVtx1, ZVtx2, TempInt);

                bRestartInterpolationAtLeftHand = false; // Restart at right hand side
            }
        }
        else if (Y0 < RenderSpec.MinClip.Y)
        {
            i32 YDiffLeft = (Y1 - Y0);
            XDeltaLeftByY = IntToFx16(X1 - X0) / YDiffLeft;
            UDeltaLeftByY = IntToFx16(UVtx1 - UVtx0) / YDiffLeft;
            VDeltaLeftByY = IntToFx16(VVtx1 - VVtx0) / YDiffLeft;
            ZDeltaLeftByY = IntToFx16(ZVtx1 - ZVtx0) / YDiffLeft;

            i32 YDiffRight = (Y2 - Y0);
            XDeltaRightByY = IntToFx16(X2 - X0) / YDiffRight;
            UDeltaRightByY = IntToFx16(UVtx2 - UVtx0) / YDiffRight;
            VDeltaRightByY = IntToFx16(VVtx2 - VVtx0) / YDiffRight;
            ZDeltaRightByY = IntToFx16(ZVtx2 - ZVtx0) / YDiffRight;

            i32 YDiff = (RenderSpec.MinClip.Y - Y0);
            XLeft = IntToFx16(X0) + YDiff * XDeltaLeftByY;
            ULeft = IntToFx16(UVtx0) + YDiff * UDeltaLeftByY;
            VLeft = IntToFx16(VVtx0) + YDiff * VDeltaLeftByY;
            ZLeft = IntToFx16(ZVtx0) + YDiff * ZDeltaLeftByY;

            XRight = IntToFx16(X0) + YDiff * XDeltaRightByY;
            URight = IntToFx16(UVtx0) + YDiff * UDeltaRightByY;
            VRight = IntToFx16(VVtx0) + YDiff * VDeltaRightByY;
            ZRight = IntToFx16(ZVtx0) + YDiff * ZDeltaRightByY;

            YStart = RenderSpec.MinClip.Y;

            /* NOTE(sean):
                Test if we need swap to keep rendering left to right.
                It can happen because we assume that
                Y1 is on left hand side and Y2 on right.
             */
            // TODO(sean): Test if we can simplify it
            if (XDeltaRightByY < XDeltaLeftByY)
            {
                VL_SWAP(XDeltaLeftByY, XDeltaRightByY, TempInt);
                VL_SWAP(UDeltaLeftByY, UDeltaRightByY, TempInt);
                VL_SWAP(VDeltaLeftByY, VDeltaRightByY, TempInt);
                VL_SWAP(ZDeltaLeftByY, ZDeltaRightByY, TempInt);

                VL_SWAP(XLeft, XRight, TempInt);
                VL_SWAP(ULeft, URight, TempInt);
                VL_SWAP(VLeft, VRight, TempInt);
                VL_SWAP(ZLeft, ZRight, TempInt);

                VL_SWAP(X1, X2, TempInt);
                VL_SWAP(Y1, Y2, TempInt);
                VL_SWAP(UVtx1, UVtx2, TempInt);
                VL_SWAP(VVtx1, VVtx2, TempInt);
                VL_SWAP(ZVtx1, ZVtx2, TempInt);

                bRestartInterpolationAtLeftHand = false; // Restart at right hand side
            }
        }
        else // No top Y clipping
        {
            i32 YDiffLeft = (Y1 - Y0);
            XDeltaLeftByY = IntToFx16(X1 - X0) / YDiffLeft;
            UDeltaLeftByY = IntToFx16(UVtx1 - UVtx0) / YDiffLeft;
            VDeltaLeftByY = IntToFx16(VVtx1 - VVtx0) / YDiffLeft;
            ZDeltaLeftByY = IntToFx16(ZVtx1 - ZVtx0) / YDiffLeft;

            i32 YDiffRight = (Y2 - Y0);
            XDeltaRightByY = IntToFx16(X2 - X0) / YDiffRight;
            UDeltaRightByY = IntToFx16(UVtx2 - UVtx0) / YDiffRight;
            VDeltaRightByY = IntToFx16(VVtx2 - VVtx0) / YDiffRight;
            ZDeltaRightByY = IntToFx16(ZVtx2 - ZVtx0) / YDiffRight;

            XRight = XLeft = IntToFx16(X0);
            URight = ULeft = IntToFx16(UVtx0);
            VRight = VLeft = IntToFx16(VVtx0);
            ZRight = ZLeft = IntToFx16(ZVtx0);

            YStart = Y0;

            /* NOTE(sean):
                Test if we need swap to keep rendering left to right.
                It can happen because we assume that
                Y1 is on left hand side and Y2 on right.
             */
            // TODO(sean): Test if we can simplify it
            if (XDeltaRightByY < XDeltaLeftByY)
            {
                VL_SWAP(XDeltaLeftByY, XDeltaRightByY, TempInt);
                VL_SWAP(UDeltaLeftByY, UDeltaRightByY, TempInt);
                VL_SWAP(VDeltaLeftByY, VDeltaRightByY, TempInt);
                VL_SWAP(ZDeltaLeftByY, ZDeltaRightByY, TempInt);

                VL_SWAP(XLeft, XRight, TempInt);
                VL_SWAP(ULeft, URight, TempInt);
                VL_SWAP(VLeft, VRight, TempInt);
                VL_SWAP(ZLeft, ZRight, TempInt);

                VL_SWAP(X1, X2, TempInt);
                VL_SWAP(Y1, Y2, TempInt);
                VL_SWAP(UVtx1, UVtx2, TempInt);
                VL_SWAP(VVtx1, VVtx2, TempInt);
                VL_SWAP(ZVtx1, ZVtx2, TempInt);

                bRestartInterpolationAtLeftHand = false; // Restart at right hand side
            }
        }

        // Test for clipping X
        if (X0 < RenderSpec.MinClip.X || X1 < RenderSpec.MinClip.X || X2 < RenderSpec.MinClip.X ||
            X0 > RenderSpec.MaxClip.X || X1 > RenderSpec.MaxClip.X || X2 > RenderSpec.MaxClip.X)
        {
            // Align video buffer
            Buffer += Pitch * YStart;
            ZBufferArray = (fx16*)ZBuffer.Buffer + (ZBuffer.Pitch * YStart);

            // Process each Y
            for (i32f Y = YStart; Y <= YEnd; ++Y)
            {
                // Compute initial values
                i32 XStart = Fx16ToIntRounded(XLeft);
                i32 XEnd   = Fx16ToIntRounded(XRight);

                fx16 U = ULeft;
                fx16 V = VLeft;
                fx16 Z = ZLeft;

                // Compute interpolants
                fx16 UDeltaByX;
                fx16 VDeltaByX;
                fx16 ZDeltaByX;

                i32 XDiff = XEnd - XStart;
                if (XDiff > 0)
                {
                    UDeltaByX = (URight - ULeft) / XDiff;
                    VDeltaByX = (VRight - VLeft) / XDiff;
                    ZDeltaByX = (ZRight - ZLeft) / XDiff;
                }
                else
                {
                    UDeltaByX = (URight - ULeft);
                    VDeltaByX = (VRight - VLeft);
                    ZDeltaByX = (ZRight - ZLeft);
                }

                // Test if we need clipping
                if (XStart < RenderSpec.MinClip.X)
                {
                    XDiff = RenderSpec.MinClip.X - XStart;
                    XStart = RenderSpec.MinClip.X;

                    U += UDeltaByX * XDiff;
                    V += VDeltaByX * XDiff;
                    Z += ZDeltaByX * XDiff;
                }
                if (XEnd > RenderSpec.MaxClip.X)
                {
                    XEnd = RenderSpec.MaxClip.X;
                }

                // Process each X
                for (i32f X = XStart; X <= XEnd; ++X)
                {
                    if (Z < ZBufferArray[X])
                    {
                        VColorARGB Pixel = TextureBuffer[Fx16ToIntRounded(V) * TexturePitch + Fx16ToIntRounded(U)];
                        Pixel.R = (Pixel.R * RBase) >> 8;
                        Pixel.G = (Pixel.G * GBase) >> 8;
                        Pixel.B = (Pixel.B * BBase) >> 8;
                        Buffer[X] = (u32)Pixel;

                        ZBufferArray[X] = Z;
                    }

                    U += UDeltaByX;
                    V += VDeltaByX;
                    Z += ZDeltaByX;
                }

                // Update values those change along Y
                XLeft += XDeltaLeftByY;
                ULeft += UDeltaLeftByY;
                VLeft += VDeltaLeftByY;
                ZLeft += ZDeltaLeftByY;

                XRight += XDeltaRightByY;
                URight += UDeltaRightByY;
                VRight += VDeltaRightByY;
                ZRight += ZDeltaRightByY;

                Buffer += Pitch;
                ZBufferArray += ZBuffer.Pitch;

                // Test for changing interpolant
                if (Y == YRestartInterpolation)
                {
                    if (bRestartInterpolationAtLeftHand)
                    {
                        // Compute new values to get from Y1 to Y2
                        i32 YDiff = (Y2 - Y1);

                        XDeltaLeftByY = IntToFx16(X2 - X1) / YDiff;
                        UDeltaLeftByY = IntToFx16(UVtx2 - UVtx1) / YDiff;
                        VDeltaLeftByY = IntToFx16(VVtx2 - VVtx1) / YDiff;
                        ZDeltaLeftByY = IntToFx16(ZVtx2 - ZVtx1) / YDiff;

                        XLeft = IntToFx16(X1);
                        ULeft = IntToFx16(UVtx1);
                        VLeft = IntToFx16(VVtx1);
                        ZLeft = IntToFx16(ZVtx1);

                        // Align down on 1 Y
                        XLeft += XDeltaLeftByY;
                        ULeft += UDeltaLeftByY;
                        VLeft += VDeltaLeftByY;
                        ZLeft += ZDeltaLeftByY;
                    }
                    else
                    {
                        // Compute new values to get from Y2 to Y1 because we swapped them
                        i32 YDiff = (Y1 - Y2);

                        XDeltaRightByY = IntToFx16(X1 - X2) / YDiff;
                        UDeltaRightByY = IntToFx16(UVtx1 - UVtx2) / YDiff;
                        VDeltaRightByY = IntToFx16(VVtx1 - VVtx2) / YDiff;
                        ZDeltaRightByY = IntToFx16(ZVtx1 - ZVtx2) / YDiff;

                        XRight = IntToFx16(X2);
                        URight = IntToFx16(UVtx2);
                        VRight = IntToFx16(VVtx2);
                        ZRight = IntToFx16(ZVtx2);

                        // Align down on 1 Y
                        XRight += XDeltaRightByY;
                        URight += UDeltaRightByY;
                        VRight += VDeltaRightByY;
                        ZRight += ZDeltaRightByY;
                    }
                }
            }
        }
        else // No X clipping
        {
            // Align video buffer
            Buffer += Pitch * YStart;
            ZBufferArray = (fx16*)ZBuffer.Buffer + (ZBuffer.Pitch * YStart);

            // Process each Y
            for (i32f Y = YStart; Y <= YEnd; ++Y)
            {
                // Compute initial values
                i32 XStart = Fx16ToIntRounded(XLeft);
                i32 XEnd   = Fx16ToIntRounded(XRight);

                fx16 U = ULeft;
                fx16 V = VLeft;
                fx16 Z = ZLeft;

                // Compute interpolants
                fx16 UDeltaByX;
                fx16 VDeltaByX;
                fx16 ZDeltaByX;

                i32 XDiff = XEnd - XStart;
                if (XDiff > 0)
                {
                    UDeltaByX = (URight - ULeft) / XDiff;
                    VDeltaByX = (VRight - VLeft) / XDiff;
                    ZDeltaByX = (ZRight - ZLeft) / XDiff;
                }
                else
                {
                    UDeltaByX = (URight - ULeft);
                    VDeltaByX = (VRight - VLeft);
                    ZDeltaByX = (ZRight - ZLeft);
                }

                // Process each X
                for (i32f X = XStart; X <= XEnd; ++X)
                {
                    if (Z < ZBufferArray[X])
                    {
                        VColorARGB Pixel = TextureBuffer[Fx16ToIntRounded(V) * TexturePitch + Fx16ToIntRounded(U)];
                        Pixel.R = (Pixel.R * RBase) >> 8;
                        Pixel.G = (Pixel.G * GBase) >> 8;
                        Pixel.B = (Pixel.B * BBase) >> 8;
                        Buffer[X] = (u32)Pixel;

                        ZBufferArray[X] = Z;
                    }

                    U += UDeltaByX;
                    V += VDeltaByX;
                    Z += ZDeltaByX;
                }

                // Update values those change along Y
                XLeft += XDeltaLeftByY;
                ULeft += UDeltaLeftByY;
                VLeft += VDeltaLeftByY;
                ZLeft += ZDeltaLeftByY;

                XRight += XDeltaRightByY;
                URight += UDeltaRightByY;
                VRight += VDeltaRightByY;
                ZRight += ZDeltaRightByY;

                Buffer += Pitch;
                ZBufferArray += ZBuffer.Pitch;

                // Test for changing interpolant
                if (Y == YRestartInterpolation)
                {
                    if (bRestartInterpolationAtLeftHand)
                    {
                        // Compute new values to get from Y1 to Y2
                        i32 YDiff = (Y2 - Y1);

                        XDeltaLeftByY = IntToFx16(X2 - X1) / YDiff;
                        UDeltaLeftByY = IntToFx16(UVtx2 - UVtx1) / YDiff;
                        VDeltaLeftByY = IntToFx16(VVtx2 - VVtx1) / YDiff;
                        ZDeltaLeftByY = IntToFx16(ZVtx2 - ZVtx1) / YDiff;

                        XLeft = IntToFx16(X1);
                        ULeft = IntToFx16(UVtx1);
                        VLeft = IntToFx16(VVtx1);
                        ZLeft = IntToFx16(ZVtx1);

                        // Align down on 1 Y
                        XLeft += XDeltaLeftByY;
                        ULeft += UDeltaLeftByY;
                        VLeft += VDeltaLeftByY;
                        ZLeft += ZDeltaLeftByY;
                    }
                    else
                    {
                        // Compute new values to get from Y2 to Y1 because we swapped them
                        i32 YDiff = (Y1 - Y2);

                        XDeltaRightByY = IntToFx16(X1 - X2) / YDiff;
                        UDeltaRightByY = IntToFx16(UVtx1 - UVtx2) / YDiff;
                        VDeltaRightByY = IntToFx16(VVtx1 - VVtx2) / YDiff;
                        ZDeltaRightByY = IntToFx16(ZVtx1 - ZVtx2) / YDiff;

                        XRight = IntToFx16(X2);
                        URight = IntToFx16(UVtx2);
                        VRight = IntToFx16(VVtx2);
                        ZRight = IntToFx16(ZVtx2);

                        // Align down on 1 Y
                        XRight += XDeltaRightByY;
                        URight += UDeltaRightByY;
                        VRight += VDeltaRightByY;
                        ZRight += ZDeltaRightByY;
                    }
                }
            }
        }
    }

    Poly.Texture->Unlock();
}

#endif // Deprecated

void VRenderer::DrawTriangle(VInterpolationContext& InterpolationContext)
{
    enum class ETriangleCase
    {
        Top,
        Bottom,
        General
    };

    u32* Buffer = InterpolationContext.Buffer;
    i32 Pitch = InterpolationContext.BufferPitch;
    VPolyFace& Poly = *InterpolationContext.Poly;

    i32 V0 = 0, V1 = 1, V2 = 2;

    // Sort by Y
    i32 TempInt;
    if (Poly.TransVtx[V1].Y < Poly.TransVtx[V0].Y)
    {
        VL_SWAP(V0, V1, TempInt);
    }
    if (Poly.TransVtx[V2].Y < Poly.TransVtx[V0].Y)
    {
        VL_SWAP(V0, V2, TempInt);
    }
    if (Poly.TransVtx[V2].Y < Poly.TransVtx[V1].Y)
    {
        VL_SWAP(V1, V2, TempInt);
    }

    // Test if we can't see it
    if (Poly.TransVtx[V2].Y < RenderSpec.MinClipFloat.Y ||
        Poly.TransVtx[V0].Y > RenderSpec.MaxClipFloat.Y ||
        (Poly.TransVtx[V0].X < RenderSpec.MinClipFloat.X && Poly.TransVtx[V1].X < RenderSpec.MinClipFloat.X && Poly.TransVtx[V2].X < RenderSpec.MinClipFloat.X) ||
        (Poly.TransVtx[V0].X > RenderSpec.MaxClipFloat.X && Poly.TransVtx[V1].X > RenderSpec.MaxClipFloat.X && Poly.TransVtx[V2].X > RenderSpec.MaxClipFloat.X))
    {
        return;
    }

    // Convert Y to integers
    i32 Y0 = (i32)(Poly.TransVtx[V0].Y + 0.5f);
    i32 Y1 = (i32)(Poly.TransVtx[V1].Y + 0.5f);
    i32 Y2 = (i32)(Poly.TransVtx[V2].Y + 0.5f);

    // Found triangle case and sort vertices by X
    ETriangleCase TriangleCase;
    if (Y0 == Y1)
    {
        if (Poly.TransVtx[V1].X < Poly.TransVtx[V0].X)
        {
            VL_SWAP(V0, V1, TempInt);
            VL_SWAP(Y0, Y1, TempInt);
        }
        TriangleCase = ETriangleCase::Top;
    }
    else if (Y1 == Y2)
    {
        if (Poly.TransVtx[V2].X < Poly.TransVtx[V1].X)
        {
            VL_SWAP(V1, V2, TempInt);
            VL_SWAP(Y1, Y2, TempInt);
        }
        TriangleCase = ETriangleCase::Bottom;
    }
    else
    {
        TriangleCase = ETriangleCase::General;
    }

    // Convert coords to integer
    i32 X0 = (i32)(Poly.TransVtx[V0].X + 0.5f);
    i32 X1 = (i32)(Poly.TransVtx[V1].X + 0.5f);
    i32 X2 = (i32)(Poly.TransVtx[V2].X + 0.5f);

    // Vertical, horizontal triangle test
    if ((Y0 == Y1 && Y1 == Y2) || (X0 == X1 && X1 == X2))
    {
        return;
    }

    // Set interpolators
    NumInterpolators = 0;

    if (Poly.Attr & EPolyAttr::ShadeModeGouraud ||
        Poly.Attr & EPolyAttr::ShadeModePhong)
    {
        Interpolators[NumInterpolators] = &GouraudInterpolator;
    }
    else
    {
        Interpolators[NumInterpolators] = &FlatInterpolator;
    }
    ++NumInterpolators;

    if (Poly.Attr & EPolyAttr::ShadeModeTexture)
    {
        // TODO(sean): Choose using distance and factors from RenderSpec
        Interpolators[NumInterpolators] = &BillinearPerspectiveTextureInterpolator;
        ++NumInterpolators;
    }

    Interpolators[NumInterpolators] = &AlphaInterpolator;
    ++NumInterpolators;

    // Start interpolators
    i32 VtxIndices[3] = { V0, V1, V2 };
    for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
    {
        Interpolators[InterpIndex]->Start(Buffer, Pitch, Poly, VtxIndices);
    }

    i32 YStart;
    i32 YEnd;

    fx28 ZVtx0 = IntToFx28(1) / (i32)(Poly.TransVtx[V0].Z + 0.5f);
    fx28 ZVtx1 = IntToFx28(1) / (i32)(Poly.TransVtx[V1].Z + 0.5f);
    fx28 ZVtx2 = IntToFx28(1) / (i32)(Poly.TransVtx[V2].Z + 0.5f);

    // Fixed coords, color channels for rasterization
    fx16 XLeft;
    fx16 XRight;
    fx28 ZLeft, ZRight;

    // Coords, colors fixed deltas by Y
    fx16 XDeltaLeftByY;
    fx28 ZDeltaLeftByY;

    fx16 XDeltaRightByY;
    fx28 ZDeltaRightByY;

    fx28* ZBufferArray;

    if (TriangleCase == ETriangleCase::Top ||
        TriangleCase == ETriangleCase::Bottom)
    {
        i32 YDiff = Y2 - Y0;

        if (TriangleCase == ETriangleCase::Top)
        {
            // Compute deltas for coords, colors
            XDeltaLeftByY = IntToFx16(X2 - X0) / YDiff;
            ZDeltaLeftByY = (ZVtx2 - ZVtx0) / YDiff;

            XDeltaRightByY = IntToFx16(X2 - X1) / YDiff;
            ZDeltaRightByY = (ZVtx2 - ZVtx1) / YDiff;

            for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
            {
                Interpolators[InterpIndex]->ComputeYStartsAndDeltas(YDiff, V0, V2, YDiff, V1, V2);
            }

            // Clipping Y
            if (Y0 < RenderSpec.MinClip.Y)
            {
                YDiff = RenderSpec.MinClip.Y - Y0;
                YStart = RenderSpec.MinClip.Y;

                XLeft = IntToFx16(X0) + YDiff * XDeltaLeftByY;
                ZLeft = (ZVtx0) + YDiff * ZDeltaLeftByY;

                XRight = IntToFx16(X1) + YDiff * XDeltaRightByY;
                ZRight = (ZVtx1) + YDiff * ZDeltaRightByY;

                for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                {
                    Interpolators[InterpIndex]->InterpolateY(YDiff, YDiff);
                }
            }
            else
            {
                YStart = Y0;

                XLeft = IntToFx16(X0);
                ZLeft = (ZVtx0);

                XRight = IntToFx16(X1);
                ZRight = (ZVtx1);
            }
        }
        else // Bottom case
        {
            // Compute deltas for coords, colors
            XDeltaLeftByY = IntToFx16(X1 - X0) / YDiff;
            ZDeltaLeftByY = (ZVtx1 - ZVtx0) / YDiff;

            XDeltaRightByY = IntToFx16(X2 - X0) / YDiff;
            ZDeltaRightByY = (ZVtx2 - ZVtx0) / YDiff;

            for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
            {
                Interpolators[InterpIndex]->ComputeYStartsAndDeltas(YDiff, V0, V1, YDiff, V0, V2);
            }

            // Clipping Y
            if (Y0 < RenderSpec.MinClip.Y)
            {
                YDiff = RenderSpec.MinClip.Y - Y0;
                YStart = RenderSpec.MinClip.Y;

                XLeft = IntToFx16(X0) + YDiff * XDeltaLeftByY;
                ZLeft = (ZVtx0) + YDiff * ZDeltaLeftByY;

                XRight = IntToFx16(X0) + YDiff * XDeltaRightByY;
                ZRight = (ZVtx0) + YDiff * ZDeltaRightByY;

                for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                {
                    Interpolators[InterpIndex]->InterpolateY(YDiff, YDiff);
                }
            }
            else
            {
                YStart = Y0;

                XLeft = IntToFx16(X0);
                ZLeft = (ZVtx0);

                XRight = IntToFx16(X0);
                ZRight = (ZVtx0);
            }
        }

        // Clip bottom Y
        if (Y2 > RenderSpec.MaxClip.Y)
        {
            YEnd = RenderSpec.MaxClip.Y + 1; // + 1 because of top-left fill convention
        }
        else
        {
            YEnd = Y2;
        }

        // Test for clipping X
        if (X0 < RenderSpec.MinClip.X || X1 < RenderSpec.MinClip.X || X2 < RenderSpec.MinClip.X ||
            X0 > RenderSpec.MaxClip.X || X1 > RenderSpec.MaxClip.X || X2 > RenderSpec.MaxClip.X)
        {
            // Align buffer pointer
            Buffer += Pitch * YStart;
            ZBufferArray = (fx28*)RenderContext.ZBuffer.Buffer + (RenderContext.ZBuffer.Pitch * YStart);

            // Process each Y
            for (i32f Y = YStart; Y < YEnd; ++Y)
            {
                // Compute starting values
                i32f XStart = Fx16ToIntRounded(XLeft);
                i32f XEnd = Fx16ToIntRounded(XRight);

                fx16 Z = ZLeft;
                fx16 ZDeltaByX;

                // Compute deltas for X interpolation
                i32f XDiff = XEnd - XStart;

                for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                {
                    Interpolators[InterpIndex]->ComputeXStartsAndDeltas(XDiff, ZLeft, ZRight);
                }

                if (XDiff > 0)
                {
                    ZDeltaByX = (ZRight - ZLeft) / XDiff;
                }
                else
                {
                    ZDeltaByX = (ZRight - ZLeft);
                }

                // X clipping
                if (XStart < RenderSpec.MinClip.X)
                {
                    i32 XDiff = RenderSpec.MinClip.X - XStart;
                    XStart = RenderSpec.MinClip.X;

                    Z += XDiff * ZDeltaByX;

                    for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                    {
                        Interpolators[InterpIndex]->InterpolateX(XDiff);
                    }
                }
                if (XEnd > RenderSpec.MaxClip.X)
                {
                    XEnd = RenderSpec.MaxClip.X + 1;
                }

                // Process each X
                for (i32f X = XStart; X < XEnd; ++X)
                {
                    if (Z > ZBufferArray[X])
                    {
                        VColorARGB FinalPixel = 0xFFFFFFFF;

                        for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                        {
                            FinalPixel = Interpolators[InterpIndex]->ProcessPixel(FinalPixel, X, Y, Z);
                        }

                        Buffer[X] = FinalPixel;

                        ZBufferArray[X] = Z;
                    }

                    // Interpolate by X
                    Z += ZDeltaByX;

                    for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                    {
                        Interpolators[InterpIndex]->InterpolateX();
                    }
                }

                // Interpolate by Y
                XLeft += XDeltaLeftByY;
                ZLeft += ZDeltaLeftByY;

                XRight += XDeltaRightByY;
                ZRight += ZDeltaRightByY;

                for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                {
                    Interpolators[InterpIndex]->InterpolateY();
                }

                Buffer += Pitch;
                ZBufferArray += RenderContext.ZBuffer.Pitch;
            }
        }
        else // Non-clipped version
        {
            // Align buffer pointer
            Buffer += Pitch * YStart;
            ZBufferArray = (fx28*)RenderContext.ZBuffer.Buffer + (RenderContext.ZBuffer.Pitch * YStart);

            // Process each Y
            for (i32f Y = YStart; Y < YEnd; ++Y)
            {
                // Compute starting values
                i32f XStart = Fx16ToIntRounded(XLeft);
                i32f XEnd = Fx16ToIntRounded(XRight);

                fx16 Z = ZLeft;
                fx16 ZDeltaByX;

                // Compute deltas for X interpolation
                i32f XDiff = XEnd - XStart;

                for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                {
                    Interpolators[InterpIndex]->ComputeXStartsAndDeltas(XDiff, ZLeft, ZRight);
                }

                if (XDiff > 0)
                {
                    ZDeltaByX = (ZRight - ZLeft) / XDiff;
                }
                else
                {
                    ZDeltaByX = (ZRight - ZLeft);
                }

                // Process each X
                for (i32f X = XStart; X < XEnd; ++X)
                {
                    if (Z > ZBufferArray[X])
                    {
                        VColorARGB FinalPixel = 0xFFFFFFFF;

                        for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                        {
                            FinalPixel = Interpolators[InterpIndex]->ProcessPixel(FinalPixel, X, Y, Z);
                        }

                        Buffer[X] = FinalPixel;

                        ZBufferArray[X] = Z;
                    }

                    // Interpolate by X
                    Z += ZDeltaByX;

                    for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                    {
                        Interpolators[InterpIndex]->InterpolateX();
                    }
                }

                // Interpolate by Y
                XLeft += XDeltaLeftByY;
                ZLeft += ZDeltaLeftByY;

                XRight += XDeltaRightByY;
                ZRight += ZDeltaRightByY;

                for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                {
                    Interpolators[InterpIndex]->InterpolateY();
                }

                Buffer += Pitch;
                ZBufferArray += RenderContext.ZBuffer.Pitch;
            }
        }
    }
    else // General case
    {
        b32 bRestartInterpolationAtLeftHand = true;
        i32 YRestartInterpolation = Y1;

        // Clip bottom Y
        if (Y2 > RenderSpec.MaxClip.Y)
        {
            YEnd = RenderSpec.MaxClip.Y + 1;
        }
        else
        {
            YEnd = Y2;
        }

        // Clip top Y
        if (Y1 < RenderSpec.MinClip.Y)
        {
            // Compute deltas
            i32 YDiffLeft = (Y2 - Y1);
            XDeltaLeftByY = IntToFx16(X2 - X1) / YDiffLeft;
            ZDeltaLeftByY = (ZVtx2 - ZVtx1) / YDiffLeft;

            i32 YDiffRight = (Y2 - Y0);
            XDeltaRightByY = IntToFx16(X2 - X0) / YDiffRight;
            ZDeltaRightByY = (ZVtx2 - ZVtx0) / YDiffRight;

            // Do clipping
            i32 YOverClipLeft = (RenderSpec.MinClip.Y - Y1);
            XLeft = IntToFx16(X1) + YOverClipLeft * XDeltaLeftByY;
            ZLeft = (ZVtx1) + YOverClipLeft * ZDeltaLeftByY;

            i32 YOverClipRight = (RenderSpec.MinClip.Y - Y0);
            XRight = IntToFx16(X0) + YOverClipRight * XDeltaRightByY;
            ZRight = (ZVtx0) + YOverClipRight * ZDeltaRightByY;

            // Do both for interpolators
            for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
            {
                Interpolators[InterpIndex]->ComputeYStartsAndDeltas(YDiffLeft, V1, V2, YDiffRight, V0, V2);
                Interpolators[InterpIndex]->InterpolateY(YOverClipLeft, YOverClipRight);
            }

            YStart = RenderSpec.MinClip.Y;

            /* NOTE(sean):
                Test if we need swap to keep rendering left to right.
                It can happen because we assume that
                Y1 is on left hand side and Y2 on right.
             */
            if (XDeltaRightByY > XDeltaLeftByY)
            {
                VL_SWAP(XDeltaLeftByY, XDeltaRightByY, TempInt);
                VL_SWAP(ZDeltaLeftByY, ZDeltaRightByY, TempInt);

                VL_SWAP(XLeft, XRight, TempInt);
                VL_SWAP(ZLeft, ZRight, TempInt);

                VL_SWAP(X1, X2, TempInt);
                VL_SWAP(Y1, Y2, TempInt);
                VL_SWAP(ZVtx1, ZVtx2, TempInt);

                for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                {
                    Interpolators[InterpIndex]->SwapLeftRight();
                }

                bRestartInterpolationAtLeftHand = false; // Restart at right hand side
            }
        }
        else if (Y0 < RenderSpec.MinClip.Y)
        {
            // Compute deltas
            i32 YDiffLeft = (Y1 - Y0);
            XDeltaLeftByY = IntToFx16(X1 - X0) / YDiffLeft;
            ZDeltaLeftByY = (ZVtx1 - ZVtx0) / YDiffLeft;

            i32 YDiffRight = (Y2 - Y0);
            XDeltaRightByY = IntToFx16(X2 - X0) / YDiffRight;
            ZDeltaRightByY = (ZVtx2 - ZVtx0) / YDiffRight;

            // Do clipping
            i32 YOverClip = (RenderSpec.MinClip.Y - Y0);
            XLeft = IntToFx16(X0) + YOverClip * XDeltaLeftByY;
            ZLeft = (ZVtx0) + YOverClip * ZDeltaLeftByY;

            XRight = IntToFx16(X0) + YOverClip * XDeltaRightByY;
            ZRight = (ZVtx0) + YOverClip * ZDeltaRightByY;

            for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
            {
                Interpolators[InterpIndex]->ComputeYStartsAndDeltas(YDiffLeft, V0, V1, YDiffRight, V0, V2);
                Interpolators[InterpIndex]->InterpolateY(YOverClip, YOverClip);
            }

            YStart = RenderSpec.MinClip.Y;

            /* NOTE(sean):
                Test if we need swap to keep rendering left to right.
                It can happen because we assume that
                Y1 is on left hand side and Y2 on right.
             */
            // TODO(sean): Test if we can simplify it
            if (XDeltaRightByY < XDeltaLeftByY)
            {
                VL_SWAP(XDeltaLeftByY, XDeltaRightByY, TempInt);
                VL_SWAP(ZDeltaLeftByY, ZDeltaRightByY, TempInt);

                VL_SWAP(XLeft, XRight, TempInt);
                VL_SWAP(ZLeft, ZRight, TempInt);

                VL_SWAP(X1, X2, TempInt);
                VL_SWAP(Y1, Y2, TempInt);
                VL_SWAP(ZVtx1, ZVtx2, TempInt);

                for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                {
                    Interpolators[InterpIndex]->SwapLeftRight();
                }

                bRestartInterpolationAtLeftHand = false; // Restart at right hand side
            }
        }
        else // No top Y clipping
        {
            i32 YDiffLeft = (Y1 - Y0);
            XDeltaLeftByY = IntToFx16(X1 - X0) / YDiffLeft;
            ZDeltaLeftByY = (ZVtx1 - ZVtx0) / YDiffLeft;

            i32 YDiffRight = (Y2 - Y0);
            XDeltaRightByY = IntToFx16(X2 - X0) / YDiffRight;
            ZDeltaRightByY = (ZVtx2 - ZVtx0) / YDiffRight;

            XRight = XLeft = IntToFx16(X0);
            ZRight = ZLeft = (ZVtx0);

            for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
            {
                Interpolators[InterpIndex]->ComputeYStartsAndDeltas(YDiffLeft, V0, V1, YDiffRight, V0, V2);
            }

            YStart = Y0;

            /* NOTE(sean):
                Test if we need swap to keep rendering left to right.
                It can happen because we assume that
                Y1 is on left hand side and Y2 on right.
             */
            if (XDeltaRightByY < XDeltaLeftByY)
            {
                VL_SWAP(XDeltaLeftByY, XDeltaRightByY, TempInt);
                VL_SWAP(ZDeltaLeftByY, ZDeltaRightByY, TempInt);

                VL_SWAP(XLeft, XRight, TempInt);
                VL_SWAP(ZLeft, ZRight, TempInt);

                VL_SWAP(X1, X2, TempInt);
                VL_SWAP(Y1, Y2, TempInt);
                VL_SWAP(ZVtx1, ZVtx2, TempInt);

                for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                {
                    Interpolators[InterpIndex]->SwapLeftRight();
                }

                bRestartInterpolationAtLeftHand = false; // Restart at right hand side
            }
        }

        // Test for clipping X
        if (X0 < RenderSpec.MinClip.X || X1 < RenderSpec.MinClip.X || X2 < RenderSpec.MinClip.X ||
            X0 > RenderSpec.MaxClip.X || X1 > RenderSpec.MaxClip.X || X2 > RenderSpec.MaxClip.X)
        {
            // Align buffer pointer
            Buffer += Pitch * YStart;
            ZBufferArray = (fx28*)RenderContext.ZBuffer.Buffer + (RenderContext.ZBuffer.Pitch * YStart);

            // Process each Y
            for (i32f Y = YStart; Y < YEnd; ++Y)
            {
                // Compute starting values
                i32f XStart = Fx16ToIntRounded(XLeft);
                i32f XEnd = Fx16ToIntRounded(XRight);

                fx16 Z = ZLeft;
                fx16 ZDeltaByX;

                // Compute deltas for X interpolation
                i32f XDiff = XEnd - XStart;

                for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                {
                    Interpolators[InterpIndex]->ComputeXStartsAndDeltas(XDiff, ZLeft, ZRight);
                }

                if (XDiff > 0)
                {
                    ZDeltaByX = (ZRight - ZLeft) / XDiff;
                }
                else
                {
                    ZDeltaByX = (ZRight - ZLeft);
                }

                // X clipping
                if (XStart < RenderSpec.MinClip.X)
                {
                    i32 XDiff = RenderSpec.MinClip.X - XStart;
                    XStart = RenderSpec.MinClip.X;

                    Z += XDiff * ZDeltaByX;

                    for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                    {
                        Interpolators[InterpIndex]->InterpolateX(XDiff);
                    }
                }
                if (XEnd > RenderSpec.MaxClip.X)
                {
                    XEnd = RenderSpec.MaxClip.X + 1;
                }

                // Process each X
                for (i32f X = XStart; X < XEnd; ++X)
                {
                    if (Z > ZBufferArray[X])
                    {
                        VColorARGB FinalPixel = 0xFFFFFFFF;

                        for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                        {
                            FinalPixel = Interpolators[InterpIndex]->ProcessPixel(FinalPixel, X, Y, Z);
                        }

                        Buffer[X] = FinalPixel;

                        ZBufferArray[X] = Z;
                    }

                    // Interpolate by X
                    Z += ZDeltaByX;

                    for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                    {
                        Interpolators[InterpIndex]->InterpolateX();
                    }
                }

                // Interpolate by Y
                XLeft += XDeltaLeftByY;
                ZLeft += ZDeltaLeftByY;

                XRight += XDeltaRightByY;
                ZRight += ZDeltaRightByY;

                for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                {
                    Interpolators[InterpIndex]->InterpolateY();
                }

                Buffer += Pitch;
                ZBufferArray += RenderContext.ZBuffer.Pitch;

                // Test for changing interpolant
                if (Y == YRestartInterpolation)
                {
                    if (bRestartInterpolationAtLeftHand)
                    {
                        // Compute new values to get from Y1 to Y2
                        i32 YDiff = (Y2 - Y1);

                        XDeltaLeftByY = IntToFx16(X2 - X1) / YDiff;
                        ZDeltaLeftByY = (ZVtx2 - ZVtx1) / YDiff;

                        XLeft = IntToFx16(X1);
                        ZLeft = (ZVtx1);

                        // Align down on 1 Y
                        XLeft += XDeltaLeftByY;
                        ZLeft += ZDeltaLeftByY;

                        // Do both for interpolators
                        for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                        {
                            Interpolators[InterpIndex]->ComputeYStartsAndDeltasLeft(YDiff, V1, V2);
                            Interpolators[InterpIndex]->InterpolateYLeft();
                        }
                    }
                    else
                    {
                        // Compute new values to get from Y2 to Y1 because we swapped them
                        i32 YDiff = (Y1 - Y2);

                        XDeltaRightByY = IntToFx16(X1 - X2) / YDiff;
                        ZDeltaRightByY = (ZVtx1 - ZVtx2) / YDiff;

                        XRight = IntToFx16(X2);
                        ZRight = (ZVtx2);

                        // Align down on 1 Y
                        XRight += XDeltaRightByY;
                        ZRight += ZDeltaRightByY;

                        // Do both for interpolators
                        for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                        {
                            Interpolators[InterpIndex]->ComputeYStartsAndDeltasRight(YDiff, V2, V1);
                            Interpolators[InterpIndex]->InterpolateYRight();
                        }
                    }
                }
            }
        }
        else // No X clipping
        {
            // Align buffer pointer
            Buffer += Pitch * YStart;
            ZBufferArray = (fx28*)RenderContext.ZBuffer.Buffer + (RenderContext.ZBuffer.Pitch * YStart);

            // Process each Y
            for (i32f Y = YStart; Y < YEnd; ++Y)
            {
                // Compute starting values
                i32f XStart = Fx16ToIntRounded(XLeft);
                i32f XEnd = Fx16ToIntRounded(XRight);

                fx16 Z = ZLeft;
                fx16 ZDeltaByX;

                // Compute deltas for X interpolation
                i32f XDiff = XEnd - XStart;

                for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                {
                    Interpolators[InterpIndex]->ComputeXStartsAndDeltas(XDiff, ZLeft, ZRight);
                }

                if (XDiff > 0)
                {
                    ZDeltaByX = (ZRight - ZLeft) / XDiff;
                }
                else
                {
                    ZDeltaByX = (ZRight - ZLeft);
                }

                // Process each X
                for (i32f X = XStart; X < XEnd; ++X)
                {
                    if (Z > ZBufferArray[X])
                    {
                        VColorARGB FinalPixel = 0xFFFFFFFF;

                        for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                        {
                            FinalPixel = Interpolators[InterpIndex]->ProcessPixel(FinalPixel, X, Y, Z);
                        }

                        Buffer[X] = FinalPixel;

                        ZBufferArray[X] = Z;
                    }

                    // Interpolate by X
                    Z += ZDeltaByX;

                    for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                    {
                        Interpolators[InterpIndex]->InterpolateX();
                    }
                }

                // Interpolate by Y
                XLeft += XDeltaLeftByY;
                ZLeft += ZDeltaLeftByY;

                XRight += XDeltaRightByY;
                ZRight += ZDeltaRightByY;

                for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                {
                    Interpolators[InterpIndex]->InterpolateY();
                }

                Buffer += Pitch;
                ZBufferArray += RenderContext.ZBuffer.Pitch;

                // Test for changing interpolant
                if (Y == YRestartInterpolation)
                {
                    if (bRestartInterpolationAtLeftHand)
                    {
                        // Compute new values to get from Y1 to Y2
                        i32 YDiff = (Y2 - Y1);

                        XDeltaLeftByY = IntToFx16(X2 - X1) / YDiff;
                        ZDeltaLeftByY = (ZVtx2 - ZVtx1) / YDiff;

                        XLeft = IntToFx16(X1);
                        ZLeft = (ZVtx1);

                        // Align down on 1 Y
                        XLeft += XDeltaLeftByY;
                        ZLeft += ZDeltaLeftByY;

                        // Do both for interpolators
                        for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                        {
                            Interpolators[InterpIndex]->ComputeYStartsAndDeltasLeft(YDiff, V1, V2);
                            Interpolators[InterpIndex]->InterpolateYLeft();
                        }
                    }
                    else
                    {
                        // Compute new values to get from Y2 to Y1 because we swapped them
                        i32 YDiff = (Y1 - Y2);

                        XDeltaRightByY = IntToFx16(X1 - X2) / YDiff;
                        ZDeltaRightByY = (ZVtx1 - ZVtx2) / YDiff;

                        XRight = IntToFx16(X2);
                        ZRight = (ZVtx2);

                        // Align down on 1 Y
                        XRight += XDeltaRightByY;
                        ZRight += ZDeltaRightByY;

                        // Do both for interpolators
                        for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
                        {
                            Interpolators[InterpIndex]->ComputeYStartsAndDeltasRight(YDiff, V2, V1);
                            Interpolators[InterpIndex]->InterpolateYRight();
                        }
                    }
                }
            }
        }
    }

    for (i32f InterpIndex = 0; InterpIndex < NumInterpolators; ++InterpIndex)
    {
        Interpolators[InterpIndex]->End();
    }
}

void VRenderer::DrawText(i32 X, i32 Y, VColorARGB Color, const char* Format, ...)
{
    // Prepare text
    static constexpr i32f TextBufferSize = 512;
    char Text[TextBufferSize];

    va_list VarList;
    va_start(VarList, Format);
    vsnprintf(Text, TextBufferSize, Format, VarList);
    va_end(VarList);

    // Convert our color
    SDL_Color SDLColor;
    SDLColor.a = Color.A;
    SDLColor.r = Color.R;
    SDLColor.g = Color.G;
    SDLColor.b = Color.B;

    // Render text
    SDL_Surface* SDLSurface = TTF_RenderText_Solid(Font, Text, SDLColor);
    VL_ASSERT(SDLSurface);
    SDL_SetColorKey(SDLSurface, SDL_TRUE, static_cast<u32*>(SDLSurface->pixels)[0]);

    // Convert surface
    SDL_Surface* SDLConverted = SDL_ConvertSurface(SDLSurface, RenderSpec.SDLPixelFormat, 0);
    VL_ASSERT(SDLConverted);

    // Blit
    SDL_Rect Dest = { X, Y, (i32f)strlen(Text) * FontCharWidth, FontCharHeight };
    SDL_BlitScaled(SDLConverted, nullptr, BackSurface.SDLSurface, &Dest);

    // Free memory
    SDL_FreeSurface(SDLSurface);
    SDL_FreeSurface(SDLConverted);
}

