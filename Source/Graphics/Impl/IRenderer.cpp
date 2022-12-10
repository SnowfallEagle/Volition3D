#include "Core/Memory.h"
#include "Graphics/Impl/IRenderer.h"

void IRenderer::DrawLine(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, u32 Color)
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

void IRenderer::DrawLineSlow(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, u32 Color)
{
    if (Math.Abs(X2 - X1) > Math.Abs(Y2 - Y1))
    {
        i32 T;
        if (X1 > X2)
        {
            SWAP(X1, X2, T);
            SWAP(Y1, Y2, T);
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
            SWAP(X1, X2, T);
            SWAP(Y1, Y2, T);
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

b32 IRenderer::ClipLine(i32& X1, i32& Y1, i32& X2, i32& Y2) const
{
    /* NOTE(sean):
        We can optimize this function if we throw away floating
        point computations, but we could loss in accuracy...
     */

    enum EClipCode
    {
        CC_N = BIT(1),
        CC_W = BIT(2),
        CC_S = BIT(3),
        CC_E = BIT(4),

        CC_NW = CC_N | CC_W,
        CC_NE = CC_N | CC_E,
        CC_SW = CC_S | CC_W,
        CC_SE = CC_S | CC_E,
    };

    i32 CX1 = X1, CY1 = Y1, CX2 = X2, CY2 = Y2;
    i32 Code1 = 0, Code2 = 0;

    // Define codes
    if (CX1 < MinClip.X)
        Code1 |= CC_W;
    else if (CX1 > MaxClip.X)
        Code1 |= CC_E;
    if (CY1 < MinClip.Y)
        Code1 |= CC_N;
    else if (CY1 > MaxClip.Y)
        Code1 |= CC_S;

    if (CX2 < MinClip.X)
        Code2 |= CC_W;
    else if (CX2 > MaxClip.X)
        Code2 |= CC_E;
    if (CY2 < MinClip.Y)
        Code2 |= CC_N;
    else if (CY2 > MaxClip.Y)
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
        CY1 = MinClip.Y;
        CX1 = (i32)(
            0.5f + (X1 + (CY1 - Y1) * (X2-X1) / (f32)(Y2-Y1))
        );
    } break;

    case CC_S:
    {
        CY1 = MaxClip.Y;
        CX1 = (i32)(
            0.5f + (X1 + (CY1 - Y1) * (X2-X1) / (f32)(Y2-Y1))
        );
    } break;

    case CC_W:
    {
        CX1 = MinClip.X;
        CY1 = (i32)(
            0.5f + (Y1 + (CX1 - X1) * (Y2-Y1) / (f32)(X2-X1))
        );
    } break;

    case CC_E:
    {
        CX1 = MaxClip.X;
        CY1 = (i32)(
            0.5f + (Y1 + (CX1 - X1) * (Y2-Y1) / (f32)(X2-X1))
        );
    } break;

    case CC_NW:
    {
        CY1 = MinClip.Y;
        CX1 = (i32)(
            0.5f + (X1 + (CY1 - Y1) * (X2-X1) / (f32)(Y2-Y1))
        );

        if (CX1 < MinClip.X || CX1 > MaxClip.X)
        {
            CX1 = MinClip.X;
            CY1 = (i32)(
                0.5f + (Y1 + (CX1 - X1) * (Y2-Y1) / (f32)(X2-X1))
            );
        }
    } break;

    case CC_NE:
    {
        CY1 = MinClip.Y;
        CX1 = (i32)(
            0.5f + (X1 + (CY1 - Y1) * (X2-X1) / (f32)(Y2-Y1))
        );

        if (CX1 < MinClip.X || CX1 > MaxClip.X)
        {
            CX1 = MaxClip.X;
            CY1 = (i32)(
                0.5f + (Y1 + (CX1 - X1) * (Y2-Y1) / (f32)(X2-X1))
            );
        }
    } break;

    case CC_SW:
    {
        CY1 = MaxClip.Y;
        CX1 = (i32)(
            0.5f + (X1 + (CY1 - Y1) * (X2-X1) / (f32)(Y2-Y1))
        );

        if (CX1 < MinClip.X || CX1 > MaxClip.X)
        {
            CX1 = MinClip.X;
            CY1 = (i32)(
                0.5f + (Y1 + (CX1 - X1) * (Y2-Y1) / (f32)(X2-X1))
            );
        }
    } break;

    case CC_SE:
    {
        CY1 = MaxClip.Y;
        CX1 = (i32)(
            0.5f + (X1 + (CY1 - Y1) * (X2-X1) / (f32)(Y2-Y1))
        );

        if (CX1 < MinClip.X || CX1 > MaxClip.X)
        {
            CX1 = MaxClip.X;
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
        CY2 = MinClip.Y;
        CX2 = (i32)(
            0.5f + (X2 + (CY2 - Y2) * (X1-X2) / (f32)(Y1-Y2))
        );
    } break;

    case CC_S:
    {
        CY2 = MaxClip.Y;
        CX2 = (i32)(
            0.5f + (X2 + (CY2 - Y2) * (X1-X2) / (f32)(Y1-Y2))
        );
    } break;

    case CC_W:
    {
        CX2 = MinClip.X;
        CY2 = (i32)(
            0.5f + (Y2 + (CX2 - X2) * (Y1-Y2) / (f32)(X1-X2))
        );
    } break;

    case CC_E:
    {
        CX2 = MaxClip.X;
        CY2 = (i32)(
            0.5f + (Y2 + (CX2 - X2) * (Y1-Y2) / (f32)(X1-X2))
        );
    } break;

    case CC_NW:
    {
        CY2 = MinClip.Y;
        CX2 = (i32)(
            0.5f + (X2 + (CY2 - Y2) * (X1-X2) / (f32)(Y1-Y2))
        );

        if (CX2 < MinClip.X || CX2 > MaxClip.X)
        {
            CX2 = MinClip.X;
            CY2 = (i32)(
                0.5f + (Y2 + (CX2 - X2) * (Y1-Y2) / (f32)(X1-X2))
            );
        }
    } break;

    case CC_NE:
    {
        CY2 = MinClip.Y;
        CX2 = (i32)(
            0.5f + (X2 + (CY2 - Y2) * (X1-X2) / (f32)(Y1-Y2))
        );

        if (CX2 < MinClip.X || CX2 > MaxClip.X)
        {
            CX2 = MaxClip.X;
            CY2 = (i32)(
                0.5f + (Y2 + (CX2 - X2) * (Y1-Y2) / (f32)(X1-X2))
            );
        }
    } break;

    case CC_SW:
    {
        CY2 = MaxClip.Y;
        CX2 = (i32)(
            0.5f + (X2 + (CY2 - Y2) * (X1-X2) / (f32)(Y1-Y2))
        );

        if (CX2 < MinClip.X || CX2 > MaxClip.X)
        {
            CX2 = MinClip.X;
            CY2 = (i32)(
                0.5f + (Y2 + (CX2 - X2) * (Y1-Y2) / (f32)(X1-X2))
            );
        }
    } break;

    case CC_SE:
    {
        CY2 = MaxClip.Y;
        CX2 = (i32)(
            0.5f + (X2 + (CY2 - Y2) * (X1-X2) / (f32)(Y1-Y2))
        );

        if (CX2 < MinClip.X || CX2 > MaxClip.X)
        {
            CX2 = MaxClip.X;
            CY2 = (i32)(
                0.5f + (Y2 + (CX2 - X2) * (Y1-Y2) / (f32)(X1-X2))
            );
        }
    } break;

    default: {} break;
    }

    // Check if we still can't see the line
    if (CX1 < MinClip.X || CX1 > MaxClip.X ||
        CX2 < MinClip.X || CX2 > MaxClip.X ||
        CY1 < MinClip.Y || CY1 > MaxClip.Y ||
        CY2 < MinClip.Y || CY2 > MaxClip.Y)
    {
        return false;
    }

    X1 = CX1;
    X2 = CX2;
    Y1 = CY1;
    Y2 = CY2;

    return true;
}

void IRenderer::DrawTopTriangle(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, i32 X3, i32 Y3, u32 Color)
{
    // Sort
    if (X1 > X2)
    {
        i32 Temp;
        SWAP(X1, X2, Temp);
    }

    f32 XStart = (f32)X1;
    f32 XEnd = (f32)X2;

    f32 OneDivHeight = 1.0f / (Y3 - Y1);
    f32 XDiffStart = (f32)(X3 - X1) * OneDivHeight;
    f32 XDiffEnd = (f32)(X3 - X2) * OneDivHeight;

    // Y clipping
    if (Y1 < MinClip.Y)
    {
        XStart += XDiffStart * (MinClip.Y - Y1);
        XEnd += XDiffEnd * (MinClip.Y - Y1);
        Y1 = MinClip.Y;
    }
    if (Y3 > MaxClip.Y)
    {
        Y3 = MaxClip.Y;
    }

    Buffer += Y1 * Pitch;

    // Test if we need X clipping
    if (X1 >= MinClip.X && X1 <= MaxClip.X &&
        X2 >= MinClip.X && X2 <= MaxClip.X &&
        X3 >= MinClip.X && X3 <= MaxClip.X)
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

            if (XClippedStart < (f32)MinClip.X)
            {
                if (XClippedEnd < (f32)MinClip.X)
                {
                    continue;
                }

                XClippedStart = (f32)MinClip.X;
            }

            if (XClippedEnd > (f32)MaxClip.X)
            {
                if (XClippedStart > (f32)MaxClip.X)
                {
                    continue;
                }

                XClippedEnd = (f32)MaxClip.X;
            }

            Memory.MemSetQuad(Buffer + (u32)XClippedStart, Color, ((i32)XClippedEnd - (i32)XClippedStart) + 1);
        }
    }
}

void IRenderer::DrawBottomTriangle(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, i32 X3, i32 Y3, u32 Color)
{
    // Sort
    if (X2 > X3)
    {
        i32 Temp;
        SWAP(X2, X3, Temp);
    }

    f32 XStart = (f32)X1;
    f32 XEnd = (f32)X1;

    f32 OneDivHeight = 1.0f / (Y3 - Y1);
    f32 XDiffStart = (f32)(X2 - X1) * OneDivHeight;
    f32 XDiffEnd = (f32)(X3 - X1) * OneDivHeight;

    // Y clipping
    if (Y1 < MinClip.Y)
    {
        XStart += XDiffStart * (MinClip.Y - Y1);
        XEnd += XDiffEnd * (MinClip.Y - Y1);
        Y1 = MinClip.Y;
    }
    if (Y3 > MaxClip.Y)
    {
        Y3 = MaxClip.Y;
    }

    Buffer += Y1 * Pitch;

    // Test if we need X clipping
    if (X1 >= MinClip.X && X1 <= MaxClip.X &&
        X2 >= MinClip.X && X2 <= MaxClip.X &&
        X3 >= MinClip.X && X3 <= MaxClip.X)
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

            if (XClippedStart < (f32)MinClip.X)
            {
                if (XClippedEnd < (f32)MinClip.X)
                {
                    continue;
                }

                XClippedStart = (f32)MinClip.X;
            }

            if (XClippedEnd > (f32)MaxClip.X)
            {
                if (XClippedStart > (f32)MaxClip.X)
                {
                    continue;
                }

                XClippedEnd = (f32)MaxClip.X;
            }

            Memory.MemSetQuad(Buffer + (u32)XClippedStart, Color, ((i32)XClippedEnd - (i32)XClippedStart) + 1);
        }
    }
}

void IRenderer::DrawTriangle(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, i32 X3, i32 Y3, u32 Color)
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
        SWAP(Y1, Y2, Temp);
        SWAP(X1, X2, Temp);
    }
    if (Y1 > Y3)
    {
        SWAP(Y1, Y3, Temp);
        SWAP(X1, X3, Temp);
    }
    if (Y2 > Y3)
    {
        SWAP(Y2, Y3, Temp);
        SWAP(X2, X3, Temp);
    }

    // Whole clipping test
    if (Y3 < MinClip.Y || Y1 > MaxClip.Y ||
        (X1 < MinClip.X && X2 < MinClip.X && X3 < MinClip.X) ||
        (X1 > MaxClip.X && X2 > MaxClip.X && X3 > MaxClip.X))
    {
        return;
    }

    if (Y1 == Y2)
    {
        DrawTopTriangle(Buffer, Pitch, X1, Y1, X2, Y2, X3, Y3, Color);
    }
    else if (Y2 == Y3)
    {
        DrawBottomTriangle(Buffer, Pitch, X1, Y1, X2, Y2, X3, Y3, Color);
    }
    else
    {
        // Find NewX which will divide triangle on Bottom/Top parts
        i32 NewX = X1 + (i32)(0.5f + (f32)(Y2 - Y1) * ((f32)(X3 - X1) / (f32)(Y3 - Y1)) );

        DrawBottomTriangle(Buffer, Pitch, X1, Y1, NewX, Y2, X2, Y2, Color);
        DrawTopTriangle(Buffer, Pitch, X2, Y2, NewX, Y2, X3, Y3, Color);
    }
}
