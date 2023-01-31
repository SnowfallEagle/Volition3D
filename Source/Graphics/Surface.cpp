#include "Graphics/Renderer.h"
#include "Graphics/Surface.h"

void VSurface::Create(SDL_Surface* InSDLSurface)
{
    VL_ASSERT(InSDLSurface);
    Destroy();

    SDLSurface = InSDLSurface;
    Width = InSDLSurface->w;
    Height = InSDLSurface->h;
}

void VSurface::Create(i32 InWidth, i32 InHeight)
{
    Destroy();

    SDL_Surface* PlatformSurface = SDL_CreateRGBSurfaceWithFormat(
        0, InWidth, InHeight, Renderer.BitsPerPixel,
        Renderer.SDLPixelFormatEnum
    );
    VL_ASSERT(PlatformSurface);

    SDLSurface = PlatformSurface;
    Width = SDLSurface->w;
    Height = SDLSurface->h;
}

void VSurface::Load(const char* Path)
{
    Destroy();

    SDL_Surface* Temp = SDL_LoadBMP(Path);
    VL_ASSERT(Temp);

    SDL_Surface* Converted = SDL_ConvertSurface(
        Temp, Renderer.SDLPixelFormat, 0
    );
    VL_ASSERT(Converted);
    SDL_FreeSurface(Temp);

    Create(Converted);
}

void VSurface::Destroy()
{
    if (SDLSurface)
    {
        SDL_FreeSurface(SDLSurface);
        SDLSurface = nullptr;
    }

    Buffer = nullptr;
    Pitch = 0;
    bLocked = false;
    Width = 0;
    Height = 0;
}

void VSurface::Lock(u32*& OutBuffer, i32& OutPitch)
{
    VL_ASSERT(!bLocked);

    /* NOTE(sean):
        We don't have to lock SDL surface
        since we don't use RLE
     */

    OutBuffer = Buffer = (u32*)SDLSurface->pixels;
    OutPitch = Pitch = SDLSurface->pitch >> 2; // Divide by 4 (Bytes per pixel)

    bLocked = true;
}

void VSurface::Unlock()
{
    VL_ASSERT(bLocked);

    bLocked = false;
}

void VSurface::FillRectHW(VRelRectI* Rect, u32 Color)
{
    // SDL_Rect has the same footprint as VRelRectI
    SDL_FillRect(SDLSurface, (SDL_Rect*)Rect, Color);
}

void VSurface::BlitHW(VRelRectI* SourceRect, VSurface* Dest, VRelRectI* DestRect)
{
    SDL_BlitScaled(SDLSurface, (SDL_Rect*)SourceRect, Dest->SDLSurface, (SDL_Rect*)DestRect);
}

#if 0
void VSurface::DrawBlended(const VRelRectI* SrcRect, VSurface* Dest, const VRelRectI* DestRect)
{
    /** TODO(sean)
        When we clip position, we need to clip width and height too and maybe we can return if W and H == 0

        Optimizations: Try 1) no floats 2) reduce count of computations
        Divide on small functions-helpers
     */

    // Compute position and size
    VVector2I SrcPos, DestPos;
    VVector2I SrcSize, DestSize;
    VVector2I SrcSurfaceSize, DestSurfaceSize;
    SrcSurfaceSize = { Width, Height };
    DestSurfaceSize = { Dest->GetWidth(), Dest->GetHeight() };

    // Source rectangle
    if (SrcRect)
    {
        SrcPos = { SrcRect->X, SrcRect->Y };
        SrcSize = { SrcRect->W, SrcRect->H };

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

    // Destination rectangle
    if (DestRect)
    {
        DestPos = { DestRect->X, DestRect->Y };
        DestSize = { DestRect->W, DestRect->H };

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
    Lock(SrcBuffer, SrcPitch);
    Dest->Lock(DestBuffer, DestPitch);

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
                MAP_XRGB32(
                    (u32)(_GET_RED(SrcPixel) * Alpha),
                    (u32)(_GET_GREEN(SrcPixel) * Alpha),
                    (u32)(_GET_BLUE(SrcPixel) * Alpha)
                )

                +

                MAP_XRGB32(
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

    Unlock();
    Dest->Unlock();
}

#endif // 0