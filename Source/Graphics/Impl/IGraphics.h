#ifndef GRAPHICS_IMPL_IGRAPHICS_H_

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
#include "Math/Math.h"
#include "Graphics/Surface.h"

/** NOTE(sean):
    No virtual functions. Speed.
 */
class IGraphics
{
public:
    static constexpr i32f BytesPerPixel = 4;
    static constexpr i32f BitsPerPixel = 32;

protected:
    VSurface* VideoSurface;
    VSurface* BackSurface;

    i32 ScreenWidth;
    i32 ScreenHeight;

public:
    // nullptr if you want to use whole surface rectangle
    void DrawSurfaceBlended(VSurface* Surface, const VRelRectI* Source, const VRelRectI* Dest)
    {
        // Compute position and size
        VVector2DI SrcPos, DestPos;
        VVector2DI SrcSize, DestSize;
        VVector2DI SrcSurfaceSize, DestSurfaceSize;
        SrcSurfaceSize = { Surface->GetWidth(), Surface->GetHeight() };
        DestSurfaceSize = { BackSurface->GetWidth(), BackSurface->GetHeight() };

        // Source
        if (Source)
        {
            SrcPos = { Source->X, Source->Y };
            SrcSize = { Source->W, Source->H };

            // Check for negative size

            // X
            if (SrcSize.X < 0)
            {
                /* NOTE(sean):
                    We want to paint this SrcPos pixel after
                    translation, so we do -= SrcSize.X + 1
                 */
                SrcPos.X += SrcSize.X + 1;
                SrcSize.X = -SrcSize.X;
            }

            // Y
            if (SrcSize.Y < 0)
            {
                SrcPos.Y += SrcSize.Y + 1;
                SrcSize.Y = -SrcSize.Y;
            }

            // Clip position

            // X
            if (SrcPos.X < 0)
                SrcPos.X = 0;
            else if (SrcPos.X > SrcSurfaceSize.X)
                SrcPos.X = SrcSurfaceSize.X - 1;

            // Y
            if (SrcPos.Y < 0)
                SrcPos.Y = 0;
            else if (SrcPos.Y > SrcSurfaceSize.Y)
                SrcPos.Y = SrcSurfaceSize.Y - 1;

            // Clip size

            // X
            if (SrcPos.X + SrcSize.X > SrcSurfaceSize.X)
                SrcSize.X = SrcSurfaceSize.X - SrcPos.X;

            // Y
            if (SrcPos.Y + SrcSize.Y > SrcSurfaceSize.Y)
                SrcSize.Y = SrcSurfaceSize.Y - SrcPos.Y;
        }
        else
        {
            SrcPos = { 0, 0 };
            SrcSize = SrcSurfaceSize;
        }

        // Destination
        if (Dest)
        {
            DestPos = { Dest->X, Dest->Y };
            DestSize = { Dest->W, Dest->H };

            // Check for negative size

            // X
            if (DestSize.X < 0)
            {
                DestPos.X += DestSize.X + 1;
                DestSize.X = -DestSize.X;
            }

            // Y
            if (DestSize.Y < 0)
            {
                DestPos.Y += DestSize.Y + 1;
                DestSize.Y = -DestSize.Y;
            }

            // Clip position

            // X
            if (DestPos.X < 0)
                DestPos.X = 0;
            else if (DestPos.X > DestSurfaceSize.X)
                DestPos.X = DestSurfaceSize.X - 1;

            // Y
            if (DestPos.Y < 0)
                DestPos.Y = 0;
            else if (DestPos.Y > DestSurfaceSize.Y)
                DestPos.Y = DestSurfaceSize.Y - 1;

            // Clip size

            // X
            if (DestPos.X + DestSize.X > DestSurfaceSize.X)
                DestSize.X = DestSurfaceSize.X - DestPos.X;

            // Y
            if (DestPos.Y + DestSize.Y > DestSurfaceSize.Y)
                DestSize.Y = DestSurfaceSize.Y - DestPos.Y;
        }
        else
        {
            DestPos = { 0, 0 };
            DestSize = DestSurfaceSize;
        }

        // Lock surfaces
        u32* SrcBuffer;
        u32* DestBuffer;
        i32 SrcPitch, DestPitch;
        Surface->Lock(SrcBuffer, SrcPitch);
        BackSurface->Lock(DestBuffer, DestPitch);

        // Prepare to blit
        SrcBuffer += SrcPos.Y * SrcPitch + SrcPos.X;
        DestBuffer += DestPos.Y * DestPitch + DestPos.X;

        f32 XScale = (f32)SrcSize.X / (f32)DestSize.X;
        f32 YScale = (f32)SrcSize.Y / (f32)DestSize.Y;

        i32f PrevYSrc = 0;

        // Blit
        for (i32f Y = 0; Y < DestSize.Y; ++Y)
        {
            for (i32f X = 0; X < DestSize.X; ++X)
            {
                u32 SrcPixel = SrcBuffer[(i32f)(X * XScale)];
                u32 DestPixel = DestBuffer[X];
                f32 Alpha = (f32)_GET_ALPHA(SrcPixel) / 255.0f;
                f32 InvAlpha = 1.0f - Alpha;

                u32 Pixel = (u32)(
                    _RGB32((u32)(_GET_RED(SrcPixel) * Alpha),
                           (u32)(_GET_GREEN(SrcPixel) * Alpha),
                           (u32)(_GET_BLUE(SrcPixel) * Alpha)) +

                    _RGB32((u32)(_GET_RED(DestPixel) * InvAlpha),
                           (u32)(_GET_GREEN(DestPixel) * InvAlpha),
                           (u32)(_GET_BLUE(DestPixel) * InvAlpha))
                );

                DestBuffer[X] = Pixel;
            }

            DestBuffer += DestPitch;

            i32f ScaledYSrc = (i32f)(Y * YScale);
            if (ScaledYSrc > PrevYSrc)
            {
                SrcBuffer += SrcPitch;
                PrevYSrc = ScaledYSrc;
            }
        }

        Surface->Unlock();
        BackSurface->Unlock();
    }

    FINLINE i32 GetScreenWidth() const
    {
        return ScreenWidth;
    }
    FINLINE i32 GetScreenHeight() const
    {
        return ScreenHeight;
    }
};

#define GRAPHICS_IMPL_IGRAPHICS_H_
#endif