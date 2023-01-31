#pragma once

#include "SDL.h"
#include "SDL_ttf.h"
#include "Engine/Core/Types.h"
#include "Engine/Core/Platform.h"
#include "Engine/Core/Assert.h"
#include "Engine/Core/Memory.h"
#include "Engine/Math/Math.h"
#include "Engine/Math/Rect.h"
#include "Engine/Math/Vector.h"
#include "Engine/Math/Fixed16.h"
#include "Engine/Graphics/RenderSpecification.h"
#include "Engine/Graphics/Surface.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Material.h"
#include "Engine/Graphics/Light.h"
#include "Engine/Graphics/Polygon.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/ZBuffer.h"
#include "Engine/Graphics/Interpolators/IInterpolator.h"
#include "Engine/Graphics/Interpolators/GouraudInterpolator.h"
#include "Engine/Graphics/Interpolators/FlatInterpolator.h"
#include "Engine/Graphics/Interpolators/BillinearPerspectiveTextureInterpolator.h"
#include "Engine/Graphics/Interpolators/AlphaInterpolator.h"

class VRenderer
{
public:
    static constexpr i32f BytesPerPixel = 4;
    static constexpr i32f BitsPerPixel = 32;
    static constexpr i32f MaxMaterials = 256;
    static constexpr i32f MaxLights = 8;
    static constexpr i32f NumInterpolators = 3;

public:
    VSurface VideoSurface;
    VSurface BackSurface;

    SDL_PixelFormat* SDLPixelFormat;
    u32 SDLPixelFormatEnum;

    i32 ScreenWidth;
    i32 ScreenHeight;

    VVector2I MinClip;
    VVector2I MaxClip;
    VVector2 MinClipFloat;
    VVector2 MaxClipFloat;

    TTF_Font* Font;
    i32 FontCharWidth; // In pixels
    i32 FontCharHeight;

    VMaterial Materials[MaxMaterials];
    i32 NumMaterials;

    VLight Lights[MaxLights];
    i32 NumLights;

    VZBuffer ZBuffer;

    IInterpolator* Interpolators[NumInterpolators];

public:
    // TODO(sean): Remove Constructor/Destructor, move code in StartUp/ShutDown
    VRenderer()
    {
        ResetMaterials();
        ResetLights();

        Interpolators[0] = new VGouraudInterpolator();
        Interpolators[1] = new VBillinearPerspectiveTextureInterpolator();
        Interpolators[2] = new VAlphaInterpolator();
    }
    ~VRenderer()
    {
        ResetMaterials();
        ResetLights();

        for (i32 I = 0; I < NumInterpolators; ++I)
        {
            delete Interpolators[I];
        }
    }

    void StartUp();
    void ShutDown();

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

    void AddLight(const VLight& InLight)
    {
        Lights[NumLights] = InLight;
        ++NumLights;
    }

    void TransformLights(const VCamera& Camera)
    {
        VMatrix44 TransMat = Camera.MatCamera;
        TransMat.C30 = TransMat.C31 = TransMat.C32 = 0.0f;

        for (i32f LightIndex = 0; LightIndex < NumLights; ++LightIndex)
        {
            VVector4::MulMat44(Lights[LightIndex].Pos, TransMat, Lights[LightIndex].TransPos);
            VVector4::MulMat44(Lights[LightIndex].Dir, TransMat, Lights[LightIndex].TransDir);
        }
    }

    void PrepareToRender()
    {
        BackSurface.FillRectHW(nullptr, MAP_XRGB32(0x00, 0x00, 0x00));
        ZBuffer.Clear();
    }
    void RenderAndFlip()
    {
        Flip();
    }

    // Very slow put pixel function to debug draw functions
    void PutPixel(u32* Buffer, i32 Pitch, i32 X, i32 Y, u32 Color) const
    {
        VL_ASSERT(X >= 0);
        VL_ASSERT(X < ScreenWidth);
        VL_ASSERT(Y >= 0);
        VL_ASSERT(Y < ScreenHeight);

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

    /*****************************************************************************************
    *   Only DrawTriangle is supported because it uses inversed z-buffer and interpolators   *
    *****************************************************************************************/

    // General case
    void DrawTriangle(u32* Buffer, i32 Pitch, const VPolyFace& Poly) const;

    // Deprecated >>>
        // Naive implementation of triangle rasterization without using top-left convention
        void DrawTopTriangleInt(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, i32 X3, i32 Y3, u32 Color) const;
        void DrawBottomTriangleInt(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, i32 X3, i32 Y3, u32 Color) const;
        void DrawTriangleInt(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, i32 X3, i32 Y3, u32 Color) const;

        // Flat emissive shade mode without z-buffer
        void DrawTriangleFloat(u32* Buffer, i32 Pitch, f32 X1, f32 Y1, f32 X2, f32 Y2, f32 X3, f32 Y3, u32 Color) const;
        void DrawTopTriangleFloat(u32* Buffer, i32 Pitch, f32 X1, f32 Y1, f32 X2, f32 Y2, f32 X3, f32 Y3, u32 Color) const;
        void DrawBottomTriangleFloat(u32* Buffer, i32 Pitch, f32 X1, f32 Y1, f32 X2, f32 Y2, f32 X3, f32 Y3, u32 Color) const;

        // For emissive and flat shade mode
        void DrawFlatTriangle(u32* Buffer, i32 Pitch, const VPolyFace& Poly) const;

        // For Gouraud shade mode
        void DrawGouraudTriangle(u32* Buffer, i32 Pitch, const VPolyFace& Poly) const;

        // For Emissive and Flat shade modes
        void DrawTexturedTriangle(u32* Buffer, i32 Pitch, const VPolyFace& Poly) const;
    // <<< Deprecated

    void DrawText(i32 X, i32 Y, VColorARGB Color, const char* Format, ...);

    VL_FINLINE i32 GetScreenWidth() const
    {
        return ScreenWidth;
    }
    VL_FINLINE i32 GetScreenHeight() const
    {
        return ScreenHeight;
    }

protected:
    void Flip();

    friend class VSurface;
};

extern VRenderer Renderer;
