/* TODO:
    Move interpolators in Material
*/

#pragma once

#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/Memory.h"
#include "Math/Math.h"
#include "Math/Rect.h"
#include "Math/Vector.h"
#include "Math/Fixed16.h"
#include "Graphics/Surface.h"
#include "Graphics/Color.h"
#include "Graphics/Material.h"
#include "Graphics/Light.h"
#include "Graphics/Polygon.h"
#include "Graphics/Camera.h"
#include "Graphics/ZBuffer.h"
#include "Graphics/Interpolator/IInterpolator.h"
#include "Graphics/Interpolator/GouraudInterpolator.h"

class IRenderer
{
public:
    static constexpr i32f BytesPerPixel = 4;
    static constexpr i32f BitsPerPixel = 32;
    static constexpr i32f MaxMaterials = 256;
    static constexpr i32f MaxLights = 8;
    static constexpr i32f NumInterpolators = 1;

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

    VZBuffer ZBuffer;

    IInterpolator* Interpolators[NumInterpolators];

public:
    IRenderer()
    {
        ResetMaterials();
        ResetLights();

        // TODO(sean): Put interpolators here
        Interpolators[0] = new VGouraudInterpolator();
    }
    virtual ~IRenderer()
    {
        ResetMaterials();
        ResetLights();

        // TODO(sean): Delete interpolators
        delete Interpolators[0];
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

    virtual void PrepareToRender()
    {
        BackSurface.FillRectHW(nullptr, MAP_XRGB32(0x00, 0x00, 0x00));
        ZBuffer.Clear();
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

    // General case
    void DrawTriangle(u32* Buffer, i32 Pitch, const VPolyFace& Poly) const;

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
