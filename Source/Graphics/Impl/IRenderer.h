#pragma once

#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/Memory.h"
#include "Math/Math.h"
#include "Math/Rect.h"
#include "Math/Vector.h"
#include "Math/Fixed16.h"
#include "Graphics/Surface.h"
#include "Graphics/Color.h"
#include "Graphics/Material.h"
#include "Graphics/Light.h"
#include "Graphics/Polygon.h"

class IRenderer
{
public:
    static constexpr i32f BytesPerPixel = 4;
    static constexpr i32f BitsPerPixel = 32;
    static constexpr i32f MaxMaterials = 256;
    static constexpr i32f MaxLights = 8;

public:
    VSurface BackSurface;

    i32 ScreenWidth;
    i32 ScreenHeight;
    VVector2I MinClip;
    VVector2I MaxClip;
    VVector2 MinClipFloat;
    VVector2 MaxClipFloat;

    VMaterial Materials[MaxMaterials];
    i32 NumMaterials;

    VLight Lights[MaxLights];
    i32 NumLights;

public:
    IRenderer()
    {
        ResetMaterials();
        ResetLights();
    }
    virtual ~IRenderer()
    {
        ResetMaterials();
        ResetLights();
    }

    virtual void StartUp() = 0;
    virtual void ShutDown() = 0;

    void ResetMaterials()
    {
        static b32 bFirstTime = true;

        if (bFirstTime)
        {
            Memory.MemSetByte(Materials, 0, sizeof(Materials));
            bFirstTime = false;
            return;
        }

        for (i32f I = 0; I < MaxMaterials; ++I)
        {
            Materials[I].Texture.Destroy();
        }
        Memory.MemSetByte(Materials, 0, sizeof(Materials));
        NumMaterials = 0;
    }

    void ResetLights()
    {
        Memory.MemSetByte(Lights, 0, sizeof(Lights));
        NumLights = 0;
    }

    void InitLight(i32 Index, const VLight& InLight)
    {
        if (Index >= 0 && Index < MaxLights)
        {
            Lights[Index] = InLight;
        }
        ++NumLights;
    }

    virtual void PrepareToRender()
    {
        BackSurface.FillRectHW(nullptr, MAP_XRGB32(0x00, 0x00, 0x00));
    }
    virtual void RenderAndFlip()
    {
        Flip();
    }

    // Very slow put pixel function to debug draw functions
    void PutPixel(u32* Buffer, i32 Pitch, i32 X, i32 Y, u32 Color) const
    {
        ASSERT(X >= 0);
        ASSERT(X < ScreenWidth);
        ASSERT(Y >= 0);
        ASSERT(Y < ScreenHeight);

        Buffer[Y*Pitch + X] = Color;
    }

    static void DrawLine(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, u32 Color);
    static void DrawLineSlow(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, u32 Color);

    b32 ClipLine(i32& X1, i32& Y1, i32& X2, i32& Y2) const;
    void DrawClippedLine(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, u32 Color) const
    {
        if (ClipLine(X1, Y1, X2, Y2))
        {
            DrawLine(Buffer, Pitch, X1, Y1, X2, Y2, Color);
        }
    }

    // Naive implementation of triangle rasterization without any convention
    void DrawTopTriangleInt(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, i32 X3, i32 Y3, u32 Color);
    void DrawBottomTriangleInt(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, i32 X3, i32 Y3, u32 Color);
    void DrawTriangleInt(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, i32 X3, i32 Y3, u32 Color);

    void DrawTopTriangle(u32* Buffer, i32 Pitch, f32 X1, f32 Y1, f32 X2, f32 Y2, f32 X3, f32 Y3, u32 Color)
    {
        // Sort
        if (X2 < X1)
        {
            f32 Temp;
            SWAP(X1, X2, Temp);
        }

        // Set Start End vectors
        f32 XStart = X1, XEnd = X2;
        i32f YStart, YEnd;

        // Compute deltas
        f32 Height = Y3 - Y1;
        f32 XDeltaStart = (X3 - X1) / Height;
        f32 XDeltaEnd   = (X3 - X2) / Height;

#if VL_RASTERIZER_MODE == VL_RASTERIZER_MODE_ACCURATE
        if (Y1 < MinClipFloat.Y)
        {
            YStart = MinClip.Y;

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

        if (Y3 > MaxClipFloat.Y)
        {
            YEnd = MaxClip.Y;
        }
        else
        {
            YEnd = (i32f)Math.Ceil(Y3) - 1;
        }
#else
        if (Y1 < MinClipFloat.Y)
        {
            f32 YDiff = (MinClipFloat.Y - Y1);
            XStart += YDiff * XDeltaStart;
            XEnd   += YDiff * XDeltaEnd;

            Y1 = MinClipFloat.Y;
        }

        if (Y3 > MaxClipFloat.Y)
        {
            Y3 = MaxClipFloat.Y + 0.5f;
        }

        YStart = (i32f)Math.Ceil(Y1);
        YEnd   = (i32f)Math.Ceil(Y3) - 1;
#endif

        Buffer += Pitch * YStart;

        // Test if we don't need X clipping
        if ((X1 >= MinClip.X && X1 <= MaxClip.X) &&
            (X2 >= MinClip.X && X2 <= MaxClip.X) &&
            (X3 >= MinClip.X && X3 <= MaxClip.X))
        {
            for (i32f Y = YStart; Y <= YEnd; ++Y, Buffer += Pitch)
            {
                Memory.MemSetQuad(Buffer + (u32)XStart, Color, (SizeType)(XEnd - XStart) + 1);
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

                if (XClippedStart < MinClipFloat.X)
                {
                    if (XClippedEnd < MinClipFloat.X)
                    {
                        continue;
                    }
                    XClippedStart = MinClipFloat.X;
                }

                if (XClippedEnd > MaxClipFloat.X)
                {
                    if (XClippedStart > MaxClipFloat.X)
                    {
                        continue;
                    }
                    XClippedEnd = MaxClipFloat.X;
                }

                Memory.MemSetQuad(Buffer + (u32)XClippedStart, Color, (SizeType)(XClippedEnd - XClippedStart) + 1);
            }
        }
    }
    void DrawBottomTriangle(u32* Buffer, i32 Pitch, f32 X1, f32 Y1, f32 X2, f32 Y2, f32 X3, f32 Y3, u32 Color)
    {
        // Sort
        if (X3 < X2)
        {
            f32 Temp;
            SWAP(X2, X3, Temp);
        }

        // Set Start End vectors
        f32 XStart = X1, XEnd = X1;
        i32f YStart, YEnd;

        // Compute deltas
        f32 Height = Y3 - Y1;
        f32 XDeltaStart = (X2 - X1) / Height;
        f32 XDeltaEnd   = (X3 - X1) / Height;

#if VL_RASTERIZER_MODE == VL_RASTERIZER_MODE_ACCURATE
        if (Y1 < MinClipFloat.Y)
        {
            YStart = MinClip.Y;

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

        if (Y3 > MaxClipFloat.Y)
        {
            YEnd = MaxClip.Y;
        }
        else
        {
            YEnd = (i32f)Math.Ceil(Y3) - 1;
        }
#else
        if (Y1 < MinClipFloat.Y)
        {
            f32 YDiff = (MinClipFloat.Y - Y1);
            XStart += YDiff * XDeltaStart;
            XEnd   += YDiff * XDeltaEnd;

            Y1 = MinClipFloat.Y;
        }

        if (Y3 > MaxClipFloat.Y)
        {
            Y3 = MaxClipFloat.Y + 0.5f;
        }

        YStart = (i32f)Math.Ceil(Y1);
        YEnd   = (i32f)Math.Ceil(Y3) - 1;
#endif

        Buffer += Pitch * YStart;

        // Test if we don't need X clipping
        if ((X1 >= MinClip.X && X1 <= MaxClip.X) &&
            (X2 >= MinClip.X && X2 <= MaxClip.X) &&
            (X3 >= MinClip.X && X3 <= MaxClip.X))
        {
            for (i32f Y = YStart; Y <= YEnd; ++Y, Buffer += Pitch)
            {
                Memory.MemSetQuad(Buffer + (u32)XStart, Color, (SizeType)(XEnd - XStart) + 1);
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

                if (XClippedStart < MinClipFloat.X)
                {
                    if (XClippedEnd < MinClipFloat.X)
                    {
                        continue;
                    }
                    XClippedStart = MinClipFloat.X;
                }

                if (XClippedEnd > MaxClipFloat.X)
                {
                    if (XClippedStart > MaxClipFloat.X)
                    {
                        continue;
                    }
                    XClippedEnd = MaxClipFloat.X;
                }

                Memory.MemSetQuad(Buffer + (u32)XClippedStart, Color, (SizeType)(XClippedEnd - XClippedStart) + 1);
            }
        }
    }
    void DrawTriangle(u32* Buffer, i32 Pitch, f32 X1, f32 Y1, f32 X2, f32 Y2, f32 X3, f32 Y3, u32 Color)
    {
        // Vertical, horizontal triangle clipping
        // FIXME(sean): Maybe killing branch prediction
        if ((Math.IsEqualFloat(X1, X2) && Math.IsEqualFloat(X2, X3)) ||
            (Math.IsEqualFloat(Y1, Y2) && Math.IsEqualFloat(Y2, Y3)))
        {
            return;
        }

        // Sort by Y
        if (Y2 < Y1)
        {
            f32 Temp;
            SWAP(X1, X2, Temp);
            SWAP(Y1, Y2, Temp);
        }
        if (Y3 < Y1)
        {
            f32 Temp;
            SWAP(X1, X3, Temp);
            SWAP(Y1, Y3, Temp);
        }
        if (Y3 < Y2)
        {
            f32 Temp;
            SWAP(X2, X3, Temp);
            SWAP(Y2, Y3, Temp);
        }

        // Screen space clipping
        if ((Y3 < MinClipFloat.Y || Y1 > MaxClipFloat.Y) ||
            (X1 < MinClipFloat.X && X2 < MinClipFloat.X && X3 < MinClipFloat.X) ||
            (X1 > MaxClipFloat.X && X2 > MaxClipFloat.X && X3 > MaxClipFloat.X))
        {
            return;
        }

        if (Math.IsEqualFloat(Y1, Y2)) // FIXME(sean): Maybe killing branch prediction
        {
            DrawTopTriangle(Buffer, Pitch, X1, Y1, X2, Y2, X3, Y3, Color);
        }
        else if (Math.IsEqualFloat(Y2, Y3)) // FIXME(sean): Maybe killing branch prediction
        {
            DrawBottomTriangle(Buffer, Pitch, X1, Y1, X2, Y2, X3, Y3, Color);
        }
        else
        {
            f32 NewX = X1 + (Y2 - Y1) * ((X3 - X1) / (Y3 - Y1));
            DrawBottomTriangle(Buffer, Pitch, X1, Y1, X2, Y2, NewX, Y2, Color);
            DrawTopTriangle(Buffer, Pitch, X2, Y2, NewX, Y2, X3, Y3, Color);
        }
    }

    void DrawGouraudTriangle(u32* Buffer, i32 Pitch, const VPolyFace& Poly)
    {
        enum class ETriangleCase
        {
            Top,
            Bottom,
            General
        };

        i32 V0 = 0, V1 = 1, V2 = 2;

        // Vertical, horizontal triangle clipping
        if ((Poly.TransVtx[V0].X == Poly.TransVtx[V1].X && Poly.TransVtx[V1].X == Poly.TransVtx[V2].X) ||
            (Poly.TransVtx[V0].Y == Poly.TransVtx[V1].Y && Poly.TransVtx[V1].Y == Poly.TransVtx[V2].Y))
        {
            return;
        }

        // Sort by Y
        i32 TempInt;
        if (Poly.TransVtx[V1].Y < Poly.TransVtx[V0].Y)
        {
            SWAP(V0, V1, TempInt);
        }
        if (Poly.TransVtx[V2].Y < Poly.TransVtx[V0].Y)
        {
            SWAP(V0, V2, TempInt);
        }
        if (Poly.TransVtx[V2].Y < Poly.TransVtx[V1].Y)
        {
            SWAP(V1, V2, TempInt);
        }

        // Test if we can't see it
        if (Poly.TransVtx[V2].Y < MinClipFloat.Y ||
            Poly.TransVtx[V0].Y > MaxClipFloat.Y ||
            (Poly.TransVtx[V0].X < MinClipFloat.X && Poly.TransVtx[V1].X < MinClipFloat.X && Poly.TransVtx[V2].X < MinClipFloat.X) ||
            (Poly.TransVtx[V0].X > MaxClipFloat.X && Poly.TransVtx[V1].X > MaxClipFloat.X && Poly.TransVtx[V2].X > MaxClipFloat.X))
        {
            return;
        }

        // Found triangle case and sort vertices by X
        ETriangleCase TriangleCase;
        if (Math.IsEqualFloat(Poly.TransVtx[V0].Y, Poly.TransVtx[V1].Y))
        {
            if (Poly.TransVtx[V1].X < Poly.TransVtx[V0].X)
            {
                SWAP(V0, V1, TempInt);
            }
            TriangleCase = ETriangleCase::Top;
        }
        else if (Math.IsEqualFloat(Poly.TransVtx[V1].Y, Poly.TransVtx[V2].Y))
        {
            if (Poly.TransVtx[V2].X < Poly.TransVtx[V1].X)
            {
                SWAP(V1, V2, TempInt);
            }
            TriangleCase = ETriangleCase::Bottom;
        }
        else
        {
            TriangleCase = ETriangleCase::General;
        }

        // Convert coords to integer
        i32 X0 = (i32)(Poly.TransVtx[V0].X + 0.5f);
        i32 Y0 = (i32)(Poly.TransVtx[V0].Y + 0.5f);

        i32 X1 = (i32)(Poly.TransVtx[V1].X + 0.5f);
        i32 Y1 = (i32)(Poly.TransVtx[V1].Y + 0.5f);

        i32 X2 = (i32)(Poly.TransVtx[V2].X + 0.5f);
        i32 Y2 = (i32)(Poly.TransVtx[V2].Y + 0.5f);

        // Fixed coords, color channels for rasterization
        fx16 XLeft, RLeft, GLeft, BLeft;
        fx16 XRight, RRight, GRight, BRight;

        // Coords, colors fixed deltas by Y
        fx16 XDeltaLeftByY;
        fx16 RDeltaLeftByY, GDeltaLeftByY, BDeltaLeftByY;

        fx16 XDeltaRightByY;
        fx16 RDeltaRightByY, GDeltaRightByY, BDeltaRightByY;

        switch (TriangleCase)
        {
        case ETriangleCase::Top:
        {
            // Compute deltas for coords, colors
            // TODO(sean): Possible optimization here
            i32 YDiff = Y2 - Y0;

            XDeltaLeftByY = IntToFx16(X2 - X0) / YDiff;
            RDeltaLeftByY = IntToFx16(Poly.LitColor[V2].R - Poly.LitColor[V0].R) / YDiff;
            GDeltaLeftByY = IntToFx16(Poly.LitColor[V2].G - Poly.LitColor[V0].G) / YDiff;
            BDeltaLeftByY = IntToFx16(Poly.LitColor[V2].B - Poly.LitColor[V0].B) / YDiff;

            XDeltaRightByY = IntToFx16(X2 - X1) / YDiff;
            RDeltaRightByY = IntToFx16(Poly.LitColor[V2].R - Poly.LitColor[V1].R) / YDiff;
            GDeltaRightByY = IntToFx16(Poly.LitColor[V2].G - Poly.LitColor[V1].G) / YDiff;
            BDeltaRightByY = IntToFx16(Poly.LitColor[V2].B - Poly.LitColor[V1].B) / YDiff;

            // Clipping Y
            if (Y0 < MinClip.Y)
            {
                // TODO(sean): Implement
                return;
            }
            else
            {
                XLeft = IntToFx16(X0);
                RLeft = IntToFx16(Poly.LitColor[V0].R);
                GLeft = IntToFx16(Poly.LitColor[V0].G);
                BLeft = IntToFx16(Poly.LitColor[V0].B);

                XRight = IntToFx16(X1);
                RRight = IntToFx16(Poly.LitColor[V1].R);
                GRight = IntToFx16(Poly.LitColor[V1].G);
                BRight = IntToFx16(Poly.LitColor[V1].B);
            }

            if (Y2 > MaxClip.Y)
            {
                Y2 = MaxClip.Y;
            }

            // Test for clipping X
            if (X0 < MinClip.X || X1 < MinClip.X || X2 < MinClip.X ||
                X0 > MaxClip.X || X1 > MaxClip.X || X2 > MaxClip.X)
            {
                // TODO(sean): Implement
                return;
            }
            else
            {
                // Align buffer pointer
                Buffer += Pitch * Y0;

                // Proccess each Y
                for (i32f Y = Y0; Y < Y2; ++Y)
                {
                    // Compute starting values
                    i32f XStart = Fx16ToIntRounded(XLeft);
                    i32f XEnd = Fx16ToIntRounded(XRight);

                    fx16 R = RLeft;
                    fx16 G = GLeft;
                    fx16 B = BLeft;

                    // Compute deltas for X interpolation
                    i32f XDiff = XEnd - XStart;

                    fx16 RDeltaByX;
                    fx16 GDeltaByX;
                    fx16 BDeltaByX;
                    if (XDiff > 0)
                    {
                        RDeltaByX = (RRight - RLeft) / XDiff;
                        GDeltaByX = (GRight - GLeft) / XDiff;
                        BDeltaByX = (BRight - BLeft) / XDiff;
                    }
                    else
                    {
                        RDeltaByX = (RRight - RLeft);
                        GDeltaByX = (GRight - GLeft);
                        BDeltaByX = (BRight - BLeft);
                    }

                    // Proccess each X
                    for (i32f X = XStart; X < XEnd; ++X)
                    {
                        Buffer[X] = MAP_XRGB32(
                            Fx16ToIntRounded(R),
                            Fx16ToIntRounded(G),
                            Fx16ToIntRounded(B)
                        );

                        // Update X values
                        R += RDeltaByX;
                        G += GDeltaByX;
                        B += BDeltaByX;
                    }

                    // Update Y values
                    XLeft += XDeltaLeftByY;
                    RLeft += RDeltaLeftByY;
                    GLeft += GDeltaLeftByY;
                    BLeft += BDeltaLeftByY;

                    XRight += XDeltaRightByY;
                    RRight += RDeltaRightByY;
                    GRight += GDeltaRightByY;
                    BRight += BDeltaRightByY;

                    Buffer += Pitch;
                }
            }
        } break;

        case ETriangleCase::Bottom:
        {

        } break;

        case ETriangleCase::General:
        {

        } break;

        default: {} break;
        }
    }

    virtual void DrawText(i32 X, i32 Y, VColorARGB Color, const char* Format, ...) = 0;

    FINLINE i32 GetScreenWidth() const
    {
        return ScreenWidth;
    }
    FINLINE i32 GetScreenHeight() const
    {
        return ScreenHeight;
    }

protected:
    virtual void Flip() = 0;
};
