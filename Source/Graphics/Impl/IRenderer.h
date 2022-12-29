#pragma once

#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/Memory.h"
#include "Math/Math.h"
#include "Math/Rect.h"
#include "Math/Vector.h"
#include "Graphics/Surface.h"
#include "Graphics/Color.h"
#include "Graphics/Material.h"
#include "Graphics/Light.h"

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
