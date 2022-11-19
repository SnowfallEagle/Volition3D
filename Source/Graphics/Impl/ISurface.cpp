#include "Math/Vector.h"
#include "Graphics/Impl/ISurface.h"

#if 0 // TODO(sean)
void ISurface::DrawSurfaceBlended(const VRelRectI* Source, u32* Buffer, i32 Pitch, const VRelRectI* Dest)
{
    /** TODO(sean)
        When we clip position, we need to clip width and height too and maybe we can return if W and H == 0

        Optimizations: Try 1) no floats 2) reduce count of computations
        Divide on small functions-helpers
     */

    // Compute position and size
    VVector2DI SrcPos, DestPos;
    VVector2DI SrcSize, DestSize;
    VVector2DI SrcSurfaceSize, DestSurfaceSize;
    SrcSurfaceSize = { Surface->GetWidth(), Surface->GetHeight() };
    DestSurfaceSize = { BackSurface.GetWidth(), BackSurface.GetHeight() };

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
    BackSurface.Lock(DestBuffer, DestPitch);

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
                _RGB32(
                    (u32)(_GET_RED(SrcPixel) * Alpha),
                    (u32)(_GET_GREEN(SrcPixel) * Alpha),
                    (u32)(_GET_BLUE(SrcPixel) * Alpha)
                )

                +

                _RGB32(
                    (u32)(_GET_RED(DestPixel) * InvAlpha),
                    (u32)(_GET_GREEN(DestPixel) * InvAlpha),
                    (u32)(_GET_BLUE(DestPixel) * InvAlpha)
                )
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
    BackSurface.Unlock();
}
#endif
