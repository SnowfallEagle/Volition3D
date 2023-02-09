#include "Engine/Core/Memory.h"
#include "Engine/Core/Window.h"
#include "Engine/Core/World.h"
#include "Engine/Graphics/Renderer.h"
#include "Engine/Graphics/RenderList.h"

VRenderer Renderer;

VL_DEFINE_LOG_CHANNEL(hLogRenderer, "Renderer");

void VRenderer::StartUp(const VRenderSpecification& InRenderSpec)
{
    // Copy render specification
    {
        RenderSpec = InRenderSpec;
    }

    // Get window surface and init pixel format
    SDL_Surface* SDLSurface;
    {
        SDLSurface = SDL_GetWindowSurface(Window.SDLWindow);
        VL_ASSERT(SDLSurface);

        RenderSpec.SDLPixelFormat = SDLSurface->format;
        RenderSpec.SDLPixelFormatEnum = RenderSpec.SDLPixelFormat->format;
    }

    // Create video and back surfaces
    {
        VideoSurface.Create(SDLSurface);

        RenderSpec.TargetSize = { VideoSurface.Width, VideoSurface.Height };

        RenderSpec.MinClip = { 0, 0 };
        RenderSpec.MaxClip = { RenderSpec.TargetSize.X - 1, RenderSpec.TargetSize.Y - 1 };

        RenderSpec.MinClipFloat = { (f32)RenderSpec.MinClip.X, (f32)RenderSpec.MinClip.Y };
        RenderSpec.MaxClipFloat = { (f32)RenderSpec.MaxClip.X, (f32)RenderSpec.MaxClip.Y };

        BackSurface.Create(RenderSpec.TargetSize.X, RenderSpec.TargetSize.Y);
    }

    // Initialize TTF
    {
        static constexpr i32f CharsPerLine = 80;
        static constexpr f32 PointDivPixel = 0.75f;
        static constexpr f32 QualityMultiplier = 2.0f;

        i32 Res = TTF_Init();
        VL_ASSERT(Res == 0);

        FontCharWidth = RenderSpec.TargetSize.X / CharsPerLine;
        FontCharHeight = (i32)(FontCharWidth * 1.25f);

        Font = TTF_OpenFont("Default.ttf", (i32)( (f32)FontCharWidth * PointDivPixel * QualityMultiplier ));
        VL_ASSERT(Font);
    }

    // Init render context and renderer stuff
    {
        RenderContext.Init();

        Memory.MemSetByte(Materials, 0, sizeof(Materials));
        ResetLights();
    }

    // Log
    VL_NOTE(hLogRenderer, "Initialized with %s pixel format\n", SDL_GetPixelFormatName(RenderSpec.SDLPixelFormatEnum));
}

void VRenderer::ShutDown()
{
    // Shut down TTF
    {
        TTF_CloseFont(Font);
        TTF_Quit();
    }

    // Free renderer stuff
    {
        ResetMaterials();
        ResetLights();

        RenderContext.Destroy();
        BackSurface.Destroy();
    }
}

void VRenderer::Flip()
{
    SDL_BlitSurface(BackSurface.SDLSurface, nullptr, VideoSurface.SDLSurface, nullptr);
    SDL_UpdateWindowSurface(Window.SDLWindow);
}

void VRenderer::DrawLine(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, u32 Color)
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

void VRenderer::DrawLineSlow(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, u32 Color)
{
    if (Math.Abs(X2 - X1) > Math.Abs(Y2 - Y1))
    {
        i32 T;
        if (X1 > X2)
        {
            VL_SWAP(X1, X2, T);
            VL_SWAP(Y1, Y2, T);
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
            VL_SWAP(X1, X2, T);
            VL_SWAP(Y1, Y2, T);
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

b32 VRenderer::ClipLine(i32& X1, i32& Y1, i32& X2, i32& Y2) const
{
    /* NOTE(sean):
        We can optimize this function if we throw away floating
        point computations, but we could loss in accuracy...
    */

    enum EClipCode
    {
        CC_N = VL_BIT(1),
        CC_W = VL_BIT(2),
        CC_S = VL_BIT(3),
        CC_E = VL_BIT(4),

        CC_NW = CC_N | CC_W,
        CC_NE = CC_N | CC_E,
        CC_SW = CC_S | CC_W,
        CC_SE = CC_S | CC_E,
    };

    i32 CX1 = X1, CY1 = Y1, CX2 = X2, CY2 = Y2;
    i32 Code1 = 0, Code2 = 0;

    // Define codes
    if (CX1 < RenderSpec.MinClip.X)
        Code1 |= CC_W;
    else if (CX1 > RenderSpec.MaxClip.X)
        Code1 |= CC_E;
    if (CY1 < RenderSpec.MinClip.Y)
        Code1 |= CC_N;
    else if (CY1 > RenderSpec.MaxClip.Y)
        Code1 |= CC_S;

    if (CX2 < RenderSpec.MinClip.X)
        Code2 |= CC_W;
    else if (CX2 > RenderSpec.MaxClip.X)
        Code2 |= CC_E;
    if (CY2 < RenderSpec.MinClip.Y)
        Code2 |= CC_N;
    else if (CY2 > RenderSpec.MaxClip.Y)
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
        CY1 = RenderSpec.MinClip.Y;
        CX1 = (i32)(
            0.5f + (X1 + (CY1 - Y1) * (X2-X1) / (f32)(Y2-Y1))
        );
    } break;

    case CC_S:
    {
        CY1 = RenderSpec.MaxClip.Y;
        CX1 = (i32)(
            0.5f + (X1 + (CY1 - Y1) * (X2-X1) / (f32)(Y2-Y1))
        );
    } break;

    case CC_W:
    {
        CX1 = RenderSpec.MinClip.X;
        CY1 = (i32)(
            0.5f + (Y1 + (CX1 - X1) * (Y2-Y1) / (f32)(X2-X1))
        );
    } break;

    case CC_E:
    {
        CX1 = RenderSpec.MaxClip.X;
        CY1 = (i32)(
            0.5f + (Y1 + (CX1 - X1) * (Y2-Y1) / (f32)(X2-X1))
        );
    } break;

    case CC_NW:
    {
        CY1 = RenderSpec.MinClip.Y;
        CX1 = (i32)(
            0.5f + (X1 + (CY1 - Y1) * (X2-X1) / (f32)(Y2-Y1))
        );

        if (CX1 < RenderSpec.MinClip.X || CX1 > RenderSpec.MaxClip.X)
        {
            CX1 = RenderSpec.MinClip.X;
            CY1 = (i32)(
                0.5f + (Y1 + (CX1 - X1) * (Y2-Y1) / (f32)(X2-X1))
            );
        }
    } break;

    case CC_NE:
    {
        CY1 = RenderSpec.MinClip.Y;
        CX1 = (i32)(
            0.5f + (X1 + (CY1 - Y1) * (X2-X1) / (f32)(Y2-Y1))
        );

        if (CX1 < RenderSpec.MinClip.X || CX1 > RenderSpec.MaxClip.X)
        {
            CX1 = RenderSpec.MaxClip.X;
            CY1 = (i32)(
                0.5f + (Y1 + (CX1 - X1) * (Y2-Y1) / (f32)(X2-X1))
            );
        }
    } break;

    case CC_SW:
    {
        CY1 = RenderSpec.MaxClip.Y;
        CX1 = (i32)(
            0.5f + (X1 + (CY1 - Y1) * (X2-X1) / (f32)(Y2-Y1))
        );

        if (CX1 < RenderSpec.MinClip.X || CX1 > RenderSpec.MaxClip.X)
        {
            CX1 = RenderSpec.MinClip.X;
            CY1 = (i32)(
                0.5f + (Y1 + (CX1 - X1) * (Y2-Y1) / (f32)(X2-X1))
            );
        }
    } break;

    case CC_SE:
    {
        CY1 = RenderSpec.MaxClip.Y;
        CX1 = (i32)(
            0.5f + (X1 + (CY1 - Y1) * (X2-X1) / (f32)(Y2-Y1))
        );

        if (CX1 < RenderSpec.MinClip.X || CX1 > RenderSpec.MaxClip.X)
        {
            CX1 = RenderSpec.MaxClip.X;
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
        CY2 = RenderSpec.MinClip.Y;
        CX2 = (i32)(
            0.5f + (X2 + (CY2 - Y2) * (X1-X2) / (f32)(Y1-Y2))
        );
    } break;

    case CC_S:
    {
        CY2 = RenderSpec.MaxClip.Y;
        CX2 = (i32)(
            0.5f + (X2 + (CY2 - Y2) * (X1-X2) / (f32)(Y1-Y2))
        );
    } break;

    case CC_W:
    {
        CX2 = RenderSpec.MinClip.X;
        CY2 = (i32)(
            0.5f + (Y2 + (CX2 - X2) * (Y1-Y2) / (f32)(X1-X2))
        );
    } break;

    case CC_E:
    {
        CX2 = RenderSpec.MaxClip.X;
        CY2 = (i32)(
            0.5f + (Y2 + (CX2 - X2) * (Y1-Y2) / (f32)(X1-X2))
        );
    } break;

    case CC_NW:
    {
        CY2 = RenderSpec.MinClip.Y;
        CX2 = (i32)(
            0.5f + (X2 + (CY2 - Y2) * (X1-X2) / (f32)(Y1-Y2))
        );

        if (CX2 < RenderSpec.MinClip.X || CX2 > RenderSpec.MaxClip.X)
        {
            CX2 = RenderSpec.MinClip.X;
            CY2 = (i32)(
                0.5f + (Y2 + (CX2 - X2) * (Y1-Y2) / (f32)(X1-X2))
            );
        }
    } break;

    case CC_NE:
    {
        CY2 = RenderSpec.MinClip.Y;
        CX2 = (i32)(
            0.5f + (X2 + (CY2 - Y2) * (X1-X2) / (f32)(Y1-Y2))
        );

        if (CX2 < RenderSpec.MinClip.X || CX2 > RenderSpec.MaxClip.X)
        {
            CX2 = RenderSpec.MaxClip.X;
            CY2 = (i32)(
                0.5f + (Y2 + (CX2 - X2) * (Y1-Y2) / (f32)(X1-X2))
            );
        }
    } break;

    case CC_SW:
    {
        CY2 = RenderSpec.MaxClip.Y;
        CX2 = (i32)(
            0.5f + (X2 + (CY2 - Y2) * (X1-X2) / (f32)(Y1-Y2))
        );

        if (CX2 < RenderSpec.MinClip.X || CX2 > RenderSpec.MaxClip.X)
        {
            CX2 = RenderSpec.MinClip.X;
            CY2 = (i32)(
                0.5f + (Y2 + (CX2 - X2) * (Y1-Y2) / (f32)(X1-X2))
            );
        }
    } break;

    case CC_SE:
    {
        CY2 = RenderSpec.MaxClip.Y;
        CX2 = (i32)(
            0.5f + (X2 + (CY2 - Y2) * (X1-X2) / (f32)(Y1-Y2))
        );

        if (CX2 < RenderSpec.MinClip.X || CX2 > RenderSpec.MaxClip.X)
        {
            CX2 = RenderSpec.MaxClip.X;
            CY2 = (i32)(
                0.5f + (Y2 + (CX2 - X2) * (Y1-Y2) / (f32)(X1-X2))
            );
        }
    } break;

    default: {} break;
    }

    // Check if we still can't see the line
    if (CX1 < RenderSpec.MinClip.X || CX1 > RenderSpec.MaxClip.X ||
        CX2 < RenderSpec.MinClip.X || CX2 > RenderSpec.MaxClip.X ||
        CY1 < RenderSpec.MinClip.Y || CY1 > RenderSpec.MaxClip.Y ||
        CY2 < RenderSpec.MinClip.Y || CY2 > RenderSpec.MaxClip.Y)
    {
        return false;
    }

    X1 = CX1;
    X2 = CX2;
    Y1 = CY1;
    Y2 = CY2;

    return true;
}

void VRenderer::DrawTriangle(VInterpolationContext& InterpolationContext)
{
    enum class ETriangleCase
    {
        Top,
        Bottom,
        General
    };

    u32* Buffer = InterpolationContext.Buffer;
    i32 Pitch = InterpolationContext.BufferPitch;

    i32 V0 = 0, V1 = 1, V2 = 2;

    // Sort by Y
    i32 TempInt;
    if (InterpolationContext.Vtx[V1].Y < InterpolationContext.Vtx[V0].Y)
    {
        VL_SWAP(V0, V1, TempInt);
    }
    if (InterpolationContext.Vtx[V2].Y < InterpolationContext.Vtx[V0].Y)
    {
        VL_SWAP(V0, V2, TempInt);
    }
    if (InterpolationContext.Vtx[V2].Y < InterpolationContext.Vtx[V1].Y)
    {
        VL_SWAP(V1, V2, TempInt);
    }

    // Test if we can't see it
    if (InterpolationContext.Vtx[V2].Y < RenderSpec.MinClipFloat.Y ||
        InterpolationContext.Vtx[V0].Y > RenderSpec.MaxClipFloat.Y ||
        (InterpolationContext.Vtx[V0].X < RenderSpec.MinClipFloat.X &&
         InterpolationContext.Vtx[V1].X < RenderSpec.MinClipFloat.X &&
         InterpolationContext.Vtx[V2].X < RenderSpec.MinClipFloat.X) ||
        (InterpolationContext.Vtx[V0].X > RenderSpec.MaxClipFloat.X &&
         InterpolationContext.Vtx[V1].X > RenderSpec.MaxClipFloat.X &&
         InterpolationContext.Vtx[V2].X > RenderSpec.MaxClipFloat.X))
    {
        return;
    }

    // Convert Y to integers
    i32 Y0 = (i32)(InterpolationContext.Vtx[V0].Y + 0.5f);
    i32 Y1 = (i32)(InterpolationContext.Vtx[V1].Y + 0.5f);
    i32 Y2 = (i32)(InterpolationContext.Vtx[V2].Y + 0.5f);

    // Found triangle case and sort vertices by X
    ETriangleCase TriangleCase;
    if (Y0 == Y1)
    {
        if (InterpolationContext.Vtx[V1].X < InterpolationContext.Vtx[V0].X)
        {
            VL_SWAP(V0, V1, TempInt);
            VL_SWAP(Y0, Y1, TempInt);
        }
        TriangleCase = ETriangleCase::Top;
    }
    else if (Y1 == Y2)
    {
        if (InterpolationContext.Vtx[V2].X < InterpolationContext.Vtx[V1].X)
        {
            VL_SWAP(V1, V2, TempInt);
            VL_SWAP(Y1, Y2, TempInt);
        }
        TriangleCase = ETriangleCase::Bottom;
    }
    else
    {
        TriangleCase = ETriangleCase::General;
    }

    // Convert coords to integer
    i32 X0 = (i32)(InterpolationContext.Vtx[V0].X + 0.5f);
    i32 X1 = (i32)(InterpolationContext.Vtx[V1].X + 0.5f);
    i32 X2 = (i32)(InterpolationContext.Vtx[V2].X + 0.5f);

    // Vertical, horizontal triangle test
    if ((Y0 == Y1 && Y1 == Y2) || (X0 == X1 && X1 == X2))
    {
        return;
    }

    InterpolationContext.VtxIndices[0] = V0;
    InterpolationContext.VtxIndices[1] = V1;
    InterpolationContext.VtxIndices[2] = V2;

    RenderContext.SetInterpolators();

    for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
    {
        RenderContext.Interpolators[InterpIndex]->SetInterpolationContext(InterpolationContext);
        RenderContext.Interpolators[InterpIndex]->Start();
    }

    i32 YStart;
    i32 YEnd;

    fx28 ZVtx0 = IntToFx28(1) / (i32)(InterpolationContext.Vtx[V0].Z + 0.5f);
    fx28 ZVtx1 = IntToFx28(1) / (i32)(InterpolationContext.Vtx[V1].Z + 0.5f);
    fx28 ZVtx2 = IntToFx28(1) / (i32)(InterpolationContext.Vtx[V2].Z + 0.5f);

    // Fixed coords, color channels for rasterization
    fx16 XLeft;
    fx16 XRight;
    fx28 ZLeft, ZRight;

    // Coords, colors fixed deltas by Y
    fx16 XDeltaLeftByY;
    fx28 ZDeltaLeftByY;

    fx16 XDeltaRightByY;
    fx28 ZDeltaRightByY;

    fx28* ZBufferArray;

    if (TriangleCase == ETriangleCase::Top ||
        TriangleCase == ETriangleCase::Bottom)
    {
        i32 YDiff = Y2 - Y0;

        if (TriangleCase == ETriangleCase::Top)
        {
            // Compute deltas for coords, colors
            XDeltaLeftByY = IntToFx16(X2 - X0) / YDiff;
            ZDeltaLeftByY = (ZVtx2 - ZVtx0) / YDiff;

            XDeltaRightByY = IntToFx16(X2 - X1) / YDiff;
            ZDeltaRightByY = (ZVtx2 - ZVtx1) / YDiff;

            for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
            {
                RenderContext.Interpolators[InterpIndex]->ComputeYStartsAndDeltas(YDiff, V0, V2, YDiff, V1, V2);
            }

            // Clipping Y
            if (Y0 < RenderSpec.MinClip.Y)
            {
                YDiff = RenderSpec.MinClip.Y - Y0;
                YStart = RenderSpec.MinClip.Y;

                XLeft = IntToFx16(X0) + YDiff * XDeltaLeftByY;
                ZLeft = (ZVtx0) + YDiff * ZDeltaLeftByY;

                XRight = IntToFx16(X1) + YDiff * XDeltaRightByY;
                ZRight = (ZVtx1) + YDiff * ZDeltaRightByY;

                for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                {
                    RenderContext.Interpolators[InterpIndex]->InterpolateY(YDiff, YDiff);
                }
            }
            else
            {
                YStart = Y0;

                XLeft = IntToFx16(X0);
                ZLeft = (ZVtx0);

                XRight = IntToFx16(X1);
                ZRight = (ZVtx1);
            }
        }
        else // Bottom case
        {
            // Compute deltas for coords, colors
            XDeltaLeftByY = IntToFx16(X1 - X0) / YDiff;
            ZDeltaLeftByY = (ZVtx1 - ZVtx0) / YDiff;

            XDeltaRightByY = IntToFx16(X2 - X0) / YDiff;
            ZDeltaRightByY = (ZVtx2 - ZVtx0) / YDiff;

            for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
            {
                RenderContext.Interpolators[InterpIndex]->ComputeYStartsAndDeltas(YDiff, V0, V1, YDiff, V0, V2);
            }

            // Clipping Y
            if (Y0 < RenderSpec.MinClip.Y)
            {
                YDiff = RenderSpec.MinClip.Y - Y0;
                YStart = RenderSpec.MinClip.Y;

                XLeft = IntToFx16(X0) + YDiff * XDeltaLeftByY;
                ZLeft = (ZVtx0) + YDiff * ZDeltaLeftByY;

                XRight = IntToFx16(X0) + YDiff * XDeltaRightByY;
                ZRight = (ZVtx0) + YDiff * ZDeltaRightByY;

                for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                {
                    RenderContext.Interpolators[InterpIndex]->InterpolateY(YDiff, YDiff);
                }
            }
            else
            {
                YStart = Y0;

                XLeft = IntToFx16(X0);
                ZLeft = (ZVtx0);

                XRight = IntToFx16(X0);
                ZRight = (ZVtx0);
            }
        }

        // Clip bottom Y
        if (Y2 > RenderSpec.MaxClip.Y)
        {
            YEnd = RenderSpec.MaxClip.Y + 1; // + 1 because of top-left fill convention
        }
        else
        {
            YEnd = Y2;
        }

        // Test for clipping X
        if (X0 < RenderSpec.MinClip.X || X1 < RenderSpec.MinClip.X || X2 < RenderSpec.MinClip.X ||
            X0 > RenderSpec.MaxClip.X || X1 > RenderSpec.MaxClip.X || X2 > RenderSpec.MaxClip.X)
        {
            // Align buffer pointer
            Buffer += Pitch * YStart;
            ZBufferArray = (fx28*)RenderContext.ZBuffer.Buffer + (RenderContext.ZBuffer.Pitch * YStart);

            // Process each Y
            for (InterpolationContext.Y = YStart; InterpolationContext.Y < YEnd; ++InterpolationContext.Y)
            {
                // Compute starting values
                i32f XStart = Fx16ToIntRounded(XLeft);
                i32f XEnd = Fx16ToIntRounded(XRight);

                fx16 Z = ZLeft;
                fx16 ZDeltaByX;

                // Compute deltas for X interpolation
                i32f XDiff = XEnd - XStart;

                for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                {
                    RenderContext.Interpolators[InterpIndex]->ComputeXStartsAndDeltas(XDiff, ZLeft, ZRight);
                }

                if (XDiff > 0)
                {
                    ZDeltaByX = (ZRight - ZLeft) / XDiff;
                }
                else
                {
                    ZDeltaByX = (ZRight - ZLeft);
                }

                // X clipping
                if (XStart < RenderSpec.MinClip.X)
                {
                    i32 XDiff = RenderSpec.MinClip.X - XStart;
                    XStart = RenderSpec.MinClip.X;

                    Z += XDiff * ZDeltaByX;

                    for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                    {
                        RenderContext.Interpolators[InterpIndex]->InterpolateX(XDiff);
                    }
                }
                if (XEnd > RenderSpec.MaxClip.X)
                {
                    XEnd = RenderSpec.MaxClip.X + 1;
                }

                // Process each X
                for (i32f X = XStart; X < XEnd; ++X)
                {
                    if (Z > ZBufferArray[X])
                    {
                        InterpolationContext.Pixel = 0xFFFFFFFF;
                        InterpolationContext.X = X;
                        InterpolationContext.Z = Z;

                        for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                        {
                            RenderContext.Interpolators[InterpIndex]->ProcessPixel();
                        }

                        Buffer[X] = InterpolationContext.Pixel;

                        ZBufferArray[X] = Z;
                    }

                    // Interpolate by X
                    Z += ZDeltaByX;

                    for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                    {
                        RenderContext.Interpolators[InterpIndex]->InterpolateX();
                    }
                }

                // Interpolate by Y
                XLeft += XDeltaLeftByY;
                ZLeft += ZDeltaLeftByY;

                XRight += XDeltaRightByY;
                ZRight += ZDeltaRightByY;

                for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                {
                    RenderContext.Interpolators[InterpIndex]->InterpolateY();
                }

                Buffer += Pitch;
                ZBufferArray += RenderContext.ZBuffer.Pitch;
            }
        }
        else // Non-clipped version
        {
            // Align buffer pointer
            Buffer += Pitch * YStart;
            ZBufferArray = (fx28*)RenderContext.ZBuffer.Buffer + (RenderContext.ZBuffer.Pitch * YStart);

            // Process each Y
            for (InterpolationContext.Y = YStart; InterpolationContext.Y < YEnd; ++InterpolationContext.Y)
            {
                // Compute starting values
                i32f XStart = Fx16ToIntRounded(XLeft);
                i32f XEnd = Fx16ToIntRounded(XRight);

                fx16 Z = ZLeft;
                fx16 ZDeltaByX;

                // Compute deltas for X interpolation
                i32f XDiff = XEnd - XStart;

                for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                {
                    RenderContext.Interpolators[InterpIndex]->ComputeXStartsAndDeltas(XDiff, ZLeft, ZRight);
                }

                if (XDiff > 0)
                {
                    ZDeltaByX = (ZRight - ZLeft) / XDiff;
                }
                else
                {
                    ZDeltaByX = (ZRight - ZLeft);
                }

                // Process each X
                for (i32f X = XStart; X < XEnd; ++X)
                {
                    if (Z > ZBufferArray[X])
                    {
                        InterpolationContext.Pixel = 0xFFFFFFFF;
                        InterpolationContext.X = X;
                        InterpolationContext.Z = Z;

                        for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                        {
                            RenderContext.Interpolators[InterpIndex]->ProcessPixel();
                        }

                        Buffer[X] = InterpolationContext.Pixel;

                        ZBufferArray[X] = Z;
                    }

                    // Interpolate by X
                    Z += ZDeltaByX;

                    for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                    {
                        RenderContext.Interpolators[InterpIndex]->InterpolateX();
                    }
                }

                // Interpolate by Y
                XLeft += XDeltaLeftByY;
                ZLeft += ZDeltaLeftByY;

                XRight += XDeltaRightByY;
                ZRight += ZDeltaRightByY;

                for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                {
                    RenderContext.Interpolators[InterpIndex]->InterpolateY();
                }

                Buffer += Pitch;
                ZBufferArray += RenderContext.ZBuffer.Pitch;
            }
        }
    }
    else // General case
    {
        b32 bRestartInterpolationAtLeftHand = true;
        i32 YRestartInterpolation = Y1;

        // Clip bottom Y
        if (Y2 > RenderSpec.MaxClip.Y)
        {
            YEnd = RenderSpec.MaxClip.Y + 1;
        }
        else
        {
            YEnd = Y2;
        }

        // Clip top Y
        if (Y1 < RenderSpec.MinClip.Y)
        {
            // Compute deltas
            i32 YDiffLeft = (Y2 - Y1);
            XDeltaLeftByY = IntToFx16(X2 - X1) / YDiffLeft;
            ZDeltaLeftByY = (ZVtx2 - ZVtx1) / YDiffLeft;

            i32 YDiffRight = (Y2 - Y0);
            XDeltaRightByY = IntToFx16(X2 - X0) / YDiffRight;
            ZDeltaRightByY = (ZVtx2 - ZVtx0) / YDiffRight;

            // Do clipping
            i32 YOverClipLeft = (RenderSpec.MinClip.Y - Y1);
            XLeft = IntToFx16(X1) + YOverClipLeft * XDeltaLeftByY;
            ZLeft = (ZVtx1) + YOverClipLeft * ZDeltaLeftByY;

            i32 YOverClipRight = (RenderSpec.MinClip.Y - Y0);
            XRight = IntToFx16(X0) + YOverClipRight * XDeltaRightByY;
            ZRight = (ZVtx0) + YOverClipRight * ZDeltaRightByY;

            // Do both for interpolators
            for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
            {
                RenderContext.Interpolators[InterpIndex]->ComputeYStartsAndDeltas(YDiffLeft, V1, V2, YDiffRight, V0, V2);
                RenderContext.Interpolators[InterpIndex]->InterpolateY(YOverClipLeft, YOverClipRight);
            }

            YStart = RenderSpec.MinClip.Y;

            /* NOTE(sean):
                Test if we need swap to keep rendering left to right.
                It can happen because we assume that
                Y1 is on left hand side and Y2 on right.
            */
            if (XDeltaRightByY > XDeltaLeftByY)
            {
                VL_SWAP(XDeltaLeftByY, XDeltaRightByY, TempInt);
                VL_SWAP(ZDeltaLeftByY, ZDeltaRightByY, TempInt);

                VL_SWAP(XLeft, XRight, TempInt);
                VL_SWAP(ZLeft, ZRight, TempInt);

                VL_SWAP(X1, X2, TempInt);
                VL_SWAP(Y1, Y2, TempInt);
                VL_SWAP(ZVtx1, ZVtx2, TempInt);

                for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                {
                    RenderContext.Interpolators[InterpIndex]->SwapLeftRight();
                }

                bRestartInterpolationAtLeftHand = false; // Restart at right hand side
            }
        }
        else if (Y0 < RenderSpec.MinClip.Y)
        {
            // Compute deltas
            i32 YDiffLeft = (Y1 - Y0);
            XDeltaLeftByY = IntToFx16(X1 - X0) / YDiffLeft;
            ZDeltaLeftByY = (ZVtx1 - ZVtx0) / YDiffLeft;

            i32 YDiffRight = (Y2 - Y0);
            XDeltaRightByY = IntToFx16(X2 - X0) / YDiffRight;
            ZDeltaRightByY = (ZVtx2 - ZVtx0) / YDiffRight;

            // Do clipping
            i32 YOverClip = (RenderSpec.MinClip.Y - Y0);
            XLeft = IntToFx16(X0) + YOverClip * XDeltaLeftByY;
            ZLeft = (ZVtx0) + YOverClip * ZDeltaLeftByY;

            XRight = IntToFx16(X0) + YOverClip * XDeltaRightByY;
            ZRight = (ZVtx0) + YOverClip * ZDeltaRightByY;

            for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
            {
                RenderContext.Interpolators[InterpIndex]->ComputeYStartsAndDeltas(YDiffLeft, V0, V1, YDiffRight, V0, V2);
                RenderContext.Interpolators[InterpIndex]->InterpolateY(YOverClip, YOverClip);
            }

            YStart = RenderSpec.MinClip.Y;

            /* NOTE(sean):
                Test if we need swap to keep rendering left to right.
                It can happen because we assume that
                Y1 is on left hand side and Y2 on right.
            */
            if (XDeltaRightByY < XDeltaLeftByY)
            {
                VL_SWAP(XDeltaLeftByY, XDeltaRightByY, TempInt);
                VL_SWAP(ZDeltaLeftByY, ZDeltaRightByY, TempInt);

                VL_SWAP(XLeft, XRight, TempInt);
                VL_SWAP(ZLeft, ZRight, TempInt);

                VL_SWAP(X1, X2, TempInt);
                VL_SWAP(Y1, Y2, TempInt);
                VL_SWAP(ZVtx1, ZVtx2, TempInt);

                for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                {
                    RenderContext.Interpolators[InterpIndex]->SwapLeftRight();
                }

                bRestartInterpolationAtLeftHand = false; // Restart at right hand side
            }
        }
        else // No top Y clipping
        {
            i32 YDiffLeft = (Y1 - Y0);
            XDeltaLeftByY = IntToFx16(X1 - X0) / YDiffLeft;
            ZDeltaLeftByY = (ZVtx1 - ZVtx0) / YDiffLeft;

            i32 YDiffRight = (Y2 - Y0);
            XDeltaRightByY = IntToFx16(X2 - X0) / YDiffRight;
            ZDeltaRightByY = (ZVtx2 - ZVtx0) / YDiffRight;

            XRight = XLeft = IntToFx16(X0);
            ZRight = ZLeft = (ZVtx0);

            for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
            {
                RenderContext.Interpolators[InterpIndex]->ComputeYStartsAndDeltas(YDiffLeft, V0, V1, YDiffRight, V0, V2);
            }

            YStart = Y0;

            /* NOTE(sean):
                Test if we need swap to keep rendering left to right.
                It can happen because we assume that
                Y1 is on left hand side and Y2 on right.
            */
            if (XDeltaRightByY < XDeltaLeftByY)
            {
                VL_SWAP(XDeltaLeftByY, XDeltaRightByY, TempInt);
                VL_SWAP(ZDeltaLeftByY, ZDeltaRightByY, TempInt);

                VL_SWAP(XLeft, XRight, TempInt);
                VL_SWAP(ZLeft, ZRight, TempInt);

                VL_SWAP(X1, X2, TempInt);
                VL_SWAP(Y1, Y2, TempInt);
                VL_SWAP(ZVtx1, ZVtx2, TempInt);

                for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                {
                    RenderContext.Interpolators[InterpIndex]->SwapLeftRight();
                }

                bRestartInterpolationAtLeftHand = false; // Restart at right hand side
            }
        }

        // Test for clipping X
        if (X0 < RenderSpec.MinClip.X || X1 < RenderSpec.MinClip.X || X2 < RenderSpec.MinClip.X ||
            X0 > RenderSpec.MaxClip.X || X1 > RenderSpec.MaxClip.X || X2 > RenderSpec.MaxClip.X)
        {
            // Align buffer pointer
            Buffer += Pitch * YStart;
            ZBufferArray = (fx28*)RenderContext.ZBuffer.Buffer + (RenderContext.ZBuffer.Pitch * YStart);

            // Process each Y
            for (InterpolationContext.Y = YStart; InterpolationContext.Y < YEnd; ++InterpolationContext.Y)
            {
                // Compute starting values
                i32f XStart = Fx16ToIntRounded(XLeft);
                i32f XEnd = Fx16ToIntRounded(XRight);

                fx16 Z = ZLeft;
                fx16 ZDeltaByX;

                // Compute deltas for X interpolation
                i32f XDiff = XEnd - XStart;

                for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                {
                    RenderContext.Interpolators[InterpIndex]->ComputeXStartsAndDeltas(XDiff, ZLeft, ZRight);
                }

                if (XDiff > 0)
                {
                    ZDeltaByX = (ZRight - ZLeft) / XDiff;
                }
                else
                {
                    ZDeltaByX = (ZRight - ZLeft);
                }

                // X clipping
                if (XStart < RenderSpec.MinClip.X)
                {
                    i32 XDiff = RenderSpec.MinClip.X - XStart;
                    XStart = RenderSpec.MinClip.X;

                    Z += XDiff * ZDeltaByX;

                    for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                    {
                        RenderContext.Interpolators[InterpIndex]->InterpolateX(XDiff);
                    }
                }
                if (XEnd > RenderSpec.MaxClip.X)
                {
                    XEnd = RenderSpec.MaxClip.X + 1;
                }

                // Process each X
                for (i32f X = XStart; X < XEnd; ++X)
                {
                    if (Z > ZBufferArray[X])
                    {
                        InterpolationContext.Pixel = 0xFFFFFFFF;
                        InterpolationContext.X = X;
                        InterpolationContext.Z = Z;

                        for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                        {
                            RenderContext.Interpolators[InterpIndex]->ProcessPixel();
                        }

                        Buffer[X] = InterpolationContext.Pixel;

                        ZBufferArray[X] = Z;
                    }

                    // Interpolate by X
                    Z += ZDeltaByX;

                    for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                    {
                        RenderContext.Interpolators[InterpIndex]->InterpolateX();
                    }
                }

                // Interpolate by Y
                XLeft += XDeltaLeftByY;
                ZLeft += ZDeltaLeftByY;

                XRight += XDeltaRightByY;
                ZRight += ZDeltaRightByY;

                for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                {
                    RenderContext.Interpolators[InterpIndex]->InterpolateY();
                }

                Buffer += Pitch;
                ZBufferArray += RenderContext.ZBuffer.Pitch;

                // Test for changing interpolant
                if (InterpolationContext.Y == YRestartInterpolation)
                {
                    if (bRestartInterpolationAtLeftHand)
                    {
                        // Compute new values to get from Y1 to Y2
                        i32 YDiff = (Y2 - Y1);

                        XDeltaLeftByY = IntToFx16(X2 - X1) / YDiff;
                        ZDeltaLeftByY = (ZVtx2 - ZVtx1) / YDiff;

                        XLeft = IntToFx16(X1);
                        ZLeft = (ZVtx1);

                        // Align down on 1 Y
                        XLeft += XDeltaLeftByY;
                        ZLeft += ZDeltaLeftByY;

                        // Do both for interpolators
                        for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                        {
                            RenderContext.Interpolators[InterpIndex]->ComputeYStartsAndDeltasLeft(YDiff, V1, V2);
                            RenderContext.Interpolators[InterpIndex]->InterpolateYLeft();
                        }
                    }
                    else
                    {
                        // Compute new values to get from Y2 to Y1 because we swapped them
                        i32 YDiff = (Y1 - Y2);

                        XDeltaRightByY = IntToFx16(X1 - X2) / YDiff;
                        ZDeltaRightByY = (ZVtx1 - ZVtx2) / YDiff;

                        XRight = IntToFx16(X2);
                        ZRight = (ZVtx2);

                        // Align down on 1 Y
                        XRight += XDeltaRightByY;
                        ZRight += ZDeltaRightByY;

                        // Do both for interpolators
                        for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                        {
                            RenderContext.Interpolators[InterpIndex]->ComputeYStartsAndDeltasRight(YDiff, V2, V1);
                            RenderContext.Interpolators[InterpIndex]->InterpolateYRight();
                        }
                    }
                }
            }
        }
        else // No X clipping
        {
            // Align buffer pointer
            Buffer += Pitch * YStart;
            ZBufferArray = (fx28*)RenderContext.ZBuffer.Buffer + (RenderContext.ZBuffer.Pitch * YStart);

            // Process each Y
            for (InterpolationContext.Y = YStart; InterpolationContext.Y < YEnd; ++InterpolationContext.Y)
            {
                // Compute starting values
                i32f XStart = Fx16ToIntRounded(XLeft);
                i32f XEnd = Fx16ToIntRounded(XRight);

                fx16 Z = ZLeft;
                fx16 ZDeltaByX;

                // Compute deltas for X interpolation
                i32f XDiff = XEnd - XStart;

                for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                {
                    RenderContext.Interpolators[InterpIndex]->ComputeXStartsAndDeltas(XDiff, ZLeft, ZRight);
                }

                if (XDiff > 0)
                {
                    ZDeltaByX = (ZRight - ZLeft) / XDiff;
                }
                else
                {
                    ZDeltaByX = (ZRight - ZLeft);
                }

                // Process each X
                for (i32f X = XStart; X < XEnd; ++X)
                {
                    if (Z > ZBufferArray[X])
                    {
                        InterpolationContext.Pixel = 0xFFFFFFFF;
                        InterpolationContext.X = X;
                        InterpolationContext.Z = Z;

                        for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                        {
                            RenderContext.Interpolators[InterpIndex]->ProcessPixel();
                        }

                        Buffer[X] = InterpolationContext.Pixel;

                        ZBufferArray[X] = Z;
                    }

                    // Interpolate by X
                    Z += ZDeltaByX;

                    for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                    {
                        RenderContext.Interpolators[InterpIndex]->InterpolateX();
                    }
                }

                // Interpolate by Y
                XLeft += XDeltaLeftByY;
                ZLeft += ZDeltaLeftByY;

                XRight += XDeltaRightByY;
                ZRight += ZDeltaRightByY;

                for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                {
                    RenderContext.Interpolators[InterpIndex]->InterpolateY();
                }

                Buffer += Pitch;
                ZBufferArray += RenderContext.ZBuffer.Pitch;

                // Test for changing interpolant
                if (InterpolationContext.Y == YRestartInterpolation)
                {
                    if (bRestartInterpolationAtLeftHand)
                    {
                        // Compute new values to get from Y1 to Y2
                        i32 YDiff = (Y2 - Y1);

                        XDeltaLeftByY = IntToFx16(X2 - X1) / YDiff;
                        ZDeltaLeftByY = (ZVtx2 - ZVtx1) / YDiff;

                        XLeft = IntToFx16(X1);
                        ZLeft = (ZVtx1);

                        // Align down on 1 Y
                        XLeft += XDeltaLeftByY;
                        ZLeft += ZDeltaLeftByY;

                        // Do both for interpolators
                        for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                        {
                            RenderContext.Interpolators[InterpIndex]->ComputeYStartsAndDeltasLeft(YDiff, V1, V2);
                            RenderContext.Interpolators[InterpIndex]->InterpolateYLeft();
                        }
                    }
                    else
                    {
                        // Compute new values to get from Y2 to Y1 because we swapped them
                        i32 YDiff = (Y1 - Y2);

                        XDeltaRightByY = IntToFx16(X1 - X2) / YDiff;
                        ZDeltaRightByY = (ZVtx1 - ZVtx2) / YDiff;

                        XRight = IntToFx16(X2);
                        ZRight = (ZVtx2);

                        // Align down on 1 Y
                        XRight += XDeltaRightByY;
                        ZRight += ZDeltaRightByY;

                        // Do both for interpolators
                        for (i32f InterpIndex = 0; InterpIndex < RenderContext.NumInterpolators; ++InterpIndex)
                        {
                            RenderContext.Interpolators[InterpIndex]->ComputeYStartsAndDeltasRight(YDiff, V2, V1);
                            RenderContext.Interpolators[InterpIndex]->InterpolateYRight();
                        }
                    }
                }
            }
        }
    }
}

void VRenderer::DrawText(i32 X, i32 Y, VColorARGB Color, const char* Format, ...)
{
    // Prepare text
    static constexpr i32f TextBufferSize = 512;
    char Text[TextBufferSize];

    va_list VarList;
    va_start(VarList, Format);
    vsnprintf(Text, TextBufferSize, Format, VarList);
    va_end(VarList);

    // Convert our color
    SDL_Color SDLColor;
    SDLColor.a = Color.A;
    SDLColor.r = Color.R;
    SDLColor.g = Color.G;
    SDLColor.b = Color.B;

    // Render text
    SDL_Surface* SDLSurface = TTF_RenderText_Solid(Font, Text, SDLColor);
    VL_ASSERT(SDLSurface);
    SDL_SetColorKey(SDLSurface, SDL_TRUE, static_cast<u32*>(SDLSurface->pixels)[0]);

    // Convert surface
    SDL_Surface* SDLConverted = SDL_ConvertSurface(SDLSurface, RenderSpec.SDLPixelFormat, 0);
    VL_ASSERT(SDLConverted);

    // Blit
    SDL_Rect Dest = { X, Y, (i32f)strlen(Text) * FontCharWidth, FontCharHeight };
    SDL_BlitScaled(SDLConverted, nullptr, BackSurface.SDLSurface, &Dest);

    // Free memory
    SDL_FreeSurface(SDLSurface);
    SDL_FreeSurface(SDLConverted);
}

