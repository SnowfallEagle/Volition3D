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

        i32 YStart;
        i32 YEnd;

        i32 RVtx0 = Poly.LitColor[V0].R, GVtx0 = Poly.LitColor[V0].G, BVtx0 = Poly.LitColor[V0].B;
        i32 RVtx1 = Poly.LitColor[V1].R, GVtx1 = Poly.LitColor[V1].G, BVtx1 = Poly.LitColor[V1].B;
        i32 RVtx2 = Poly.LitColor[V2].R, GVtx2 = Poly.LitColor[V2].G, BVtx2 = Poly.LitColor[V2].B;

        // Fixed coords, color channels for rasterization
        fx16 XLeft, RLeft, GLeft, BLeft;
        fx16 XRight, RRight, GRight, BRight;

        // Coords, colors fixed deltas by Y
        fx16 XDeltaLeftByY;
        fx16 RDeltaLeftByY, GDeltaLeftByY, BDeltaLeftByY;

        fx16 XDeltaRightByY;
        fx16 RDeltaRightByY, GDeltaRightByY, BDeltaRightByY;

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

                XDeltaRightByY = IntToFx16(X2 - X1) / YDiff;
                RDeltaRightByY = IntToFx16(RVtx2 - RVtx1) / YDiff;
                GDeltaRightByY = IntToFx16(GVtx2 - GVtx1) / YDiff;
                BDeltaRightByY = IntToFx16(BVtx2 - BVtx1) / YDiff;

                // Clipping Y
                if (Y0 < MinClip.Y)
                {
                    YDiff = MinClip.Y - Y0;
                    YStart = MinClip.Y;

                    XLeft = IntToFx16(X0) + YDiff * XDeltaLeftByY;
                    RLeft = IntToFx16(RVtx0) + YDiff * RDeltaLeftByY;
                    GLeft = IntToFx16(GVtx0) + YDiff * GDeltaLeftByY;
                    BLeft = IntToFx16(BVtx0) + YDiff * BDeltaLeftByY;

                    XRight = IntToFx16(X1) + YDiff * XDeltaRightByY;
                    RRight = IntToFx16(RVtx1) + YDiff * RDeltaRightByY;
                    GRight = IntToFx16(GVtx1) + YDiff * GDeltaRightByY;
                    BRight = IntToFx16(BVtx1) + YDiff * BDeltaRightByY;
                }
                else
                {
                    YStart = Y0;

                    XLeft = IntToFx16(X0);
                    RLeft = IntToFx16(RVtx0);
                    GLeft = IntToFx16(GVtx0);
                    BLeft = IntToFx16(BVtx0);

                    XRight = IntToFx16(X1);
                    RRight = IntToFx16(RVtx1);
                    GRight = IntToFx16(GVtx1);
                    BRight = IntToFx16(BVtx1);
                }
            }
            else // Bottom case
            {
                // Compute deltas for coords, colors
                XDeltaLeftByY = IntToFx16(X1 - X0) / YDiff;
                RDeltaLeftByY = IntToFx16(RVtx1 - RVtx0) / YDiff;
                GDeltaLeftByY = IntToFx16(GVtx1 - GVtx0) / YDiff;
                BDeltaLeftByY = IntToFx16(BVtx1 - BVtx0) / YDiff;

                XDeltaRightByY = IntToFx16(X2 - X0) / YDiff;
                RDeltaRightByY = IntToFx16(RVtx2 - RVtx0) / YDiff;
                GDeltaRightByY = IntToFx16(GVtx2 - GVtx0) / YDiff;
                BDeltaRightByY = IntToFx16(BVtx2 - BVtx0) / YDiff;

                // Clipping Y
                if (Y0 < MinClip.Y)
                {
                    YDiff = MinClip.Y - Y0;
                    YStart = MinClip.Y;

                    XLeft = IntToFx16(X0) + YDiff * XDeltaLeftByY;
                    RLeft = IntToFx16(RVtx0) + YDiff * RDeltaLeftByY;
                    GLeft = IntToFx16(GVtx0) + YDiff * GDeltaLeftByY;
                    BLeft = IntToFx16(BVtx0) + YDiff * BDeltaLeftByY;

                    XRight = IntToFx16(X0) + YDiff * XDeltaRightByY;
                    RRight = IntToFx16(RVtx0) + YDiff * RDeltaRightByY;
                    GRight = IntToFx16(GVtx0) + YDiff * GDeltaRightByY;
                    BRight = IntToFx16(BVtx0) + YDiff * BDeltaRightByY;
                }
                else
                {
                    YStart = Y0;

                    XLeft = IntToFx16(X0);
                    RLeft = IntToFx16(RVtx0);
                    GLeft = IntToFx16(GVtx0);
                    BLeft = IntToFx16(BVtx0);

                    XRight = IntToFx16(X0);
                    RRight = IntToFx16(RVtx0);
                    GRight = IntToFx16(GVtx0);
                    BRight = IntToFx16(BVtx0);
                }
            }

            // Clip bottom Y
            if (Y2 > MaxClip.Y)
            {
                YEnd = MaxClip.Y;
            }
            else
            {
                YEnd = Y2;
            }

            // Test for clipping X
            if (X0 < MinClip.X || X1 < MinClip.X || X2 < MinClip.X ||
                X0 > MaxClip.X || X1 > MaxClip.X || X2 > MaxClip.X)
            {
                // Align buffer pointer
                Buffer += Pitch * YStart;

                // Proccess each Y
                for (i32f Y = YStart; Y <= YEnd; ++Y)
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

                    // X clipping
                    if (XStart < MinClip.X)
                    {
                        i32 XDiff = MinClip.X - XStart;
                        XStart = MinClip.X;

                        R += XDiff * RDeltaByX;
                        G += XDiff * GDeltaByX;
                        B += XDiff * BDeltaByX;
                    }
                    if (XEnd > MaxClip.X)
                    {
                        XEnd = MaxClip.X;
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
            else // Non-clipped version
            {
                // Align buffer pointer
                Buffer += Pitch * YStart;

                // Proccess each Y
                for (i32f Y = YStart; Y <= YEnd; ++Y)
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
                    for (i32f X = XStart; X <= XEnd; ++X)
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
        }
        else // General case
        {
            b32 bRestartInterpolationAtLeftHand = true;
            i32 YRestartInterpolation = Y1;

            // Clip bottom Y
            if (Y2 > MaxClip.Y)
            {
                YEnd = MaxClip.Y;
            }
            else
            {
                YEnd = Y2;
            }

            // Clip top Y
            if (Y1 < MinClip.Y)
            {
                // TODO(sean)
            }
            else if (Y0 < MinClip.Y)
            {
                // TODO(sean)
            }
            else // No top Y clipping
            {
                i32 YDiffLeft = (Y1 - Y0);
                XDeltaLeftByY = IntToFx16(X1 - X0) / YDiffLeft;
                RDeltaLeftByY = IntToFx16(RVtx1 - RVtx0) / YDiffLeft;
                GDeltaLeftByY = IntToFx16(GVtx1 - GVtx0) / YDiffLeft;
                BDeltaLeftByY = IntToFx16(BVtx1 - BVtx0) / YDiffLeft;

                i32 YDiffRight = (Y2 - Y0);
                XDeltaRightByY = IntToFx16(X2 - X0) / YDiffRight;
                RDeltaRightByY = IntToFx16(RVtx2 - RVtx0) / YDiffRight;
                GDeltaRightByY = IntToFx16(GVtx2 - GVtx0) / YDiffRight;
                BDeltaRightByY = IntToFx16(BVtx2 - BVtx0) / YDiffRight;

                XRight = XLeft = IntToFx16(X0);
                RRight = RLeft = IntToFx16(RVtx0);
                GRight = GLeft = IntToFx16(GVtx0);
                BRight = BLeft = IntToFx16(BVtx0);

                YStart = Y0;

                /* NOTE(sean):
                    Test if we need swap to keep rendering left to right.
                    It can happen because we assume that
                    Y1 is on left hand side and Y2 on right.
                 */
                // TODO(sean): Test if we can simplify it
                if (XDeltaRightByY < XDeltaLeftByY)
                {
                    SWAP(XDeltaLeftByY, XDeltaRightByY, TempInt);
                    SWAP(RDeltaLeftByY, RDeltaRightByY, TempInt);
                    SWAP(GDeltaLeftByY, GDeltaRightByY, TempInt);
                    SWAP(BDeltaLeftByY, BDeltaRightByY, TempInt);

                    SWAP(XLeft, XRight, TempInt);
                    SWAP(RLeft, RRight, TempInt);
                    SWAP(GLeft, GRight, TempInt);
                    SWAP(BLeft, BRight, TempInt);

                    SWAP(X1, X2, TempInt);
                    SWAP(Y1, Y2, TempInt);
                    SWAP(RVtx1, RVtx2, TempInt);
                    SWAP(GVtx1, GVtx2, TempInt);
                    SWAP(BVtx1, BVtx2, TempInt);

                    bRestartInterpolationAtLeftHand = false; // Restart at right hand side
                }

                // Test for clipping X
                if (X0 < MinClip.X || X1 < MinClip.X || X2 < MinClip.X ||
                    X0 > MaxClip.X || X1 > MaxClip.X || X2 > MaxClip.X)
                {
                    // TODO(sean)
                }
                else
                {
                    // Align video buffer
                    Buffer += Pitch * YStart;

                    // Proccess each Y
                    for (i32f Y = YStart; Y <= YEnd; ++Y)
                    {
                        // Compute initial values
                        i32 XStart = Fx16ToIntRounded(XLeft);
                        i32 XEnd   = Fx16ToIntRounded(XRight);

                        fx16 R = RLeft;
                        fx16 G = GLeft;
                        fx16 B = BLeft;

                        // Compute interpolants
                        fx16 RDeltaByX;
                        fx16 GDeltaByX;
                        fx16 BDeltaByX;

                        i32 XDiff = XEnd - XStart;
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
                        for (i32f X = XStart; X <= XEnd; ++X)
                        {
                            Buffer[X] = MAP_XRGB32(
                                Fx16ToIntRounded(R),
                                Fx16ToIntRounded(G),
                                Fx16ToIntRounded(B)
                            );

                            R += RDeltaByX;
                            G += GDeltaByX;
                            B += BDeltaByX;
                        }

                        // Update values those change along Y
                        XLeft += XDeltaLeftByY;
                        RLeft += RDeltaLeftByY;
                        GLeft += GDeltaLeftByY;
                        BLeft += BDeltaLeftByY;

                        XRight += XDeltaRightByY;
                        RRight += RDeltaRightByY;
                        GRight += GDeltaRightByY;
                        BRight += BDeltaRightByY;

                        Buffer += Pitch;

                        // Test for changing interpolant
                        if (Y == YRestartInterpolation)
                        {
                            if (bRestartInterpolationAtLeftHand)
                            {
                                // Compute new values to get from Y1 to Y2
                                i32 YDiff = (Y2 - Y1); // FIXME(sean): Can we get situation where YDiff == 0?

                                XDeltaLeftByY = IntToFx16(X2 - X1) / YDiff;
                                RDeltaLeftByY = IntToFx16(RVtx2 - RVtx1) / YDiff;
                                GDeltaLeftByY = IntToFx16(GVtx2 - GVtx1) / YDiff;
                                BDeltaLeftByY = IntToFx16(BVtx2 - BVtx1) / YDiff;

                                XLeft = IntToFx16(X1);
                                RLeft = IntToFx16(RVtx1);
                                GLeft = IntToFx16(GVtx1);
                                BLeft = IntToFx16(BVtx1);

                                // Align down on 1 Y
                                XLeft += XDeltaLeftByY;
                                RLeft += RDeltaLeftByY;
                                GLeft += GDeltaLeftByY;
                                BLeft += BDeltaLeftByY;
                            }
                            else
                            {
                                // Compute new values to get from Y2 to Y1 because we swapped them
                                i32 YDiff = (Y1 - Y2); // FIXME(sean): Can we get situation where YDiff == 0?

                                XDeltaRightByY = IntToFx16(X1 - X2) / YDiff;
                                RDeltaRightByY = IntToFx16(RVtx1 - RVtx2) / YDiff;
                                GDeltaRightByY = IntToFx16(GVtx1 - GVtx2) / YDiff;
                                BDeltaRightByY = IntToFx16(BVtx1 - BVtx2) / YDiff;

                                XRight = IntToFx16(X2);
                                RRight = IntToFx16(RVtx2);
                                GRight = IntToFx16(GVtx2);
                                BRight = IntToFx16(BVtx2);

                                // Align down on 1 Y
                                XRight += XDeltaRightByY;
                                RRight += RDeltaRightByY;
                                GRight += GDeltaRightByY;
                                BRight += BDeltaRightByY;
                            }
                        }
                    }
                }
            }
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
