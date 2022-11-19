#pragma once

// Macroses for fast mapping ARGB32/XRGB32 format
#define _ALPHA_SHIFT (24)
#define _RED_SHIFT (16)
#define _GREEN_SHIFT (8)
#define _BLUE_SHIFT (0)

#define _RGBA32(A, R, G, B) ( ((A) << _ALPHA_SHIFT) | ((R) << _RED_SHIFT) | ((G) << _GREEN_SHIFT) | ((B)) << _BLUE_SHIFT )
#define _RGB32(R, G, B) _RGBA32(0, R, G, B)

#define _GET_ALPHA(COLOR) ( ((COLOR) >> _ALPHA_SHIFT) & 0xFF )
#define _GET_RED(COLOR) ( ((COLOR) >> _RED_SHIFT) & 0xFF )
#define _GET_GREEN(COLOR) ( ((COLOR) >> _GREEN_SHIFT) & 0xFF )
#define _GET_BLUE(COLOR) ( ((COLOR) >> _BLUE_SHIFT ) & 0xFF )

#include "Core/Types.h"
#include "Core/Platform.h"
#include "Math/Rect.h"
#include "Graphics/Surface.h"
#include "Math/Math.h"

class IRenderer
{
public:
    static constexpr i32f BytesPerPixel = 4;
    static constexpr i32f BitsPerPixel = 32;

public: // TODO(sean): For faster testing
    VSurface BackSurface;

protected:
    VSurface VideoSurface;
    i32 ScreenWidth;
    i32 ScreenHeight;

public:
    virtual void PrepareToRender()
    {
        BackSurface.FillRectHW(nullptr, _RGB32(0x00, 0x00, 0x00));
    }
    virtual void RenderAndFlip()
    {
        Flip();
    }

    // Very slow put pixel function to debug draw functions
    void PutPixel(u32* Buffer, i32 Pitch, i32 X, i32 Y, u32 Color)
    {
        ASSERT(X >= 0);
        ASSERT(X < ScreenWidth);
        ASSERT(Y >= 0);
        ASSERT(Y < ScreenHeight);

        Buffer[Y*Pitch + X] = Color;
    }
    static void DrawLine(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, u32 Color)
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
