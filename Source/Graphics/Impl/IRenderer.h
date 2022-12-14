#pragma once

// Macroses for fast mapping ARGB32/XRGB32 format
#define _ALPHA_SHIFT (24)
#define _RED_SHIFT (16)
#define _GREEN_SHIFT (8)
#define _BLUE_SHIFT (0)

#define _GET_ALPHA(COLOR) ( ((COLOR) >> _ALPHA_SHIFT) & 0xFF )
#define _GET_RED(COLOR) ( ((COLOR) >> _RED_SHIFT) & 0xFF )
#define _GET_GREEN(COLOR) ( ((COLOR) >> _GREEN_SHIFT) & 0xFF )
#define _GET_BLUE(COLOR) ( ((COLOR) >> _BLUE_SHIFT ) & 0xFF )

#define MAP_ARGB32(A, R, G, B) ( ((A) << _ALPHA_SHIFT) | ((R) << _RED_SHIFT) | ((G) << _GREEN_SHIFT) | ((B)) << _BLUE_SHIFT )
#define MAP_XRGB32(R, G, B) MAP_ARGB32(0, R, G, B)

#include "Core/Types.h"
#include "Core/Platform.h"
#include "Math/Math.h"
#include "Math/Rect.h"
#include "Math/Vector.h"
#include "Graphics/Surface.h"

class IRenderer
{
public:
    static constexpr i32f BytesPerPixel = 4;
    static constexpr i32f BitsPerPixel = 32;

public: // TODO(sean): For faster testing
    VSurface BackSurface;

protected:
    i32 ScreenWidth;
    i32 ScreenHeight;
    VVector2DI MinClip;
    VVector2DI MaxClip;
    VVector2D MinClipFloat;
    VVector2D MaxClipFloat;

public:
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
            DrawLine(Buffer, Pitch, X1, Y1, X2, Y2, Color);
    }

    void DrawTopTriangle(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, i32 X3, i32 Y3, u32 Color);
    void DrawBottomTriangle(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, i32 X3, i32 Y3, u32 Color);
    void DrawTriangle(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, i32 X3, i32 Y3, u32 Color);

    virtual void DrawText(i32 X, i32 Y, u32 Color, const char* Format, ...) = 0;

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
