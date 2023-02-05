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
#include "Engine/Graphics/RenderContext.h"
#include "Engine/Graphics/Interpolators/IInterpolator.h"
#include "Engine/Graphics/Interpolators/FlatInterpolator.h"
/* TODO(sean)
#include "Engine/Graphics/Interpolators/GouraudInterpolator.h"
#include "Engine/Graphics/Interpolators/BillinearPerspectiveTextureInterpolator.h"
#include "Engine/Graphics/Interpolators/PerspectiveCorrectTextureInterpolator.h"
#include "Engine/Graphics/Interpolators/LinearPiecewiseTextureInterpolator.h"
#include "Engine/Graphics/Interpolators/TextureInterpolator.h" // TODO(sean): Rename in affine texture interpolator
#include "Engine/Graphics/Interpolators/AlphaInterpolator.h"
*/

class VRenderer
{
public:
    static constexpr i32f BytesPerPixel = 4; // TODO(sean): -> RenderSpec
    static constexpr i32f BitsPerPixel = 32; // TODO(sean): -> RenderSpec

    // TODO(sean): Remove it later
    static constexpr i32f MaxMaterials = 256;
    static constexpr i32f MaxLights = 8;
    static constexpr i32f MaxInterpolators = 8;

private:
    VSurface VideoSurface;
    VSurface BackSurface;

    VRenderSpecification RenderSpec;
    VRenderContext RenderContext;

    // TODO(sean): Do we need support multiple fonts?
    TTF_Font* Font;
    i32 FontCharWidth; // In pixels
    i32 FontCharHeight;

    VMaterial Materials[MaxMaterials];
    i32 NumMaterials;

    // TODO(sean): Later we could put light as entity in world
    VLight Lights[MaxLights];
    i32 NumLights;

    IInterpolator* Interpolators[MaxInterpolators];
    i32 NumInterpolators;

    VFlatInterpolator FlatInterpolator;
    /* TODO(sean)
    VGouraudInterpolator GouraudInterpolator;
    VBillinearPerspectiveTextureInterpolator BillinearPerspectiveTextureInterpolator;
    VPerspectiveCorrectTextureInterpolator PerspectiveCorrectTextureInterpolator;
    VLinearPiecewiseTextureInterpolator LinearPiecewiseTextureInterpolator;
    VTextureInterpolator AffineTextureInterpolator;
    VAlphaInterpolator AlphaInterpolator;
    */

public:
    // TODO(sean): Remove Constructor/Destructor, move code in StartUp/ShutDown
    VRenderer() :
        RenderContext(RenderSpec)
    {
        ResetMaterials();
        ResetLights();
    }
    ~VRenderer()
    {
        ResetMaterials();
        ResetLights();
    }

    void StartUp(const VRenderSpecification& InRenderSpec);
    void ShutDown();

    VL_FINLINE const VRenderSpecification& GetRenderSpec() const
    {
        return RenderSpec;
    }

    VL_FINLINE i32 GetScreenWidth() const
    {
        return RenderSpec.TargetSize.X;
    }
    VL_FINLINE i32 GetScreenHeight() const
    {
        return RenderSpec.TargetSize.Y;
    }

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

    VL_FINLINE void PrepareToRender()
    {
        BackSurface.FillRectHW(nullptr, MAP_XRGB32(0x00, 0x00, 0x00));
        RenderContext.PrepareToRender();
    }

    VL_FINLINE void RenderWorld()
    {
        u32* Buffer;
        i32 Pitch;
        BackSurface.Lock(Buffer, Pitch);

        RenderContext.RenderWorld(Buffer, Pitch);

        BackSurface.Unlock();
    }

    void Flip();

    // Very slow put pixel function to debug draw functions
    void PutPixel(u32* Buffer, i32 Pitch, i32 X, i32 Y, u32 Color) const
    {
        VL_ASSERT(X >= 0);
        VL_ASSERT(X < RenderSpec.TargetSize.X);
        VL_ASSERT(Y >= 0);
        VL_ASSERT(Y < RenderSpec.TargetSize.Y);

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
    void DrawTriangle(VInterpolationContext& InterpolationContext);

    /* Deprecated >> >
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
    <<< Deprecated */

    void DrawText(i32 X, i32 Y, VColorARGB Color, const char* Format, ...);

    friend class VSurface;
    friend class VMesh;
    friend class VRenderContext;
};

extern VRenderer Renderer;
