#pragma once

/* TODO(sean):
    - Think about our functions that want surface to be locked...
        Maybe we should make functions with buffer and pitch as arguments?
 */

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

class IRenderer
{
public:
    static constexpr i32f BytesPerPixel = 4;
    static constexpr i32f BitsPerPixel = 32;

protected:
    VSurface VideoSurface;
    VSurface BackSurface;

    i32 ScreenWidth;
    i32 ScreenHeight;

public:
    virtual void PrepareToRender()
    {
        FillRect(&BackSurface, nullptr, _RGB32(0XFF, 0x00, 0x00));
    }
    virtual void RenderAndFlip()
    {
        Flip();
    }

    virtual void FillRect(VSurface* Dest, VRelRectI* Rect, u32 Color) = 0;
    virtual void DrawText(i32 X, i32 Y, u32 Color, const char* Format, ...) = 0;
    // TODO(sean): Put this in VSurface and don't use lock-based blitting??
    void DrawSurfaceBlended(VSurface* Surface, const VRelRectI* Source, const VRelRectI* Dest);

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
