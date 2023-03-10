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

namespace Volition
{

class VRenderer
{
public:
    // TODO(sean): Remove it later
    static constexpr i32f MaxMaterials = 256;
    static constexpr i32f MaxLights = 8;

private:
    VSurface VideoSurface;
    VSurface BackSurface;

    VRenderSpecification RenderSpec;
    VRenderContext RenderContext; // TODO(sean): Remove RenderContext

    TTF_Font* Font;
    i32 FontCharWidth; // In pixels
    i32 FontCharHeight;

    VMaterial Materials[MaxMaterials];
    i32 NumMaterials;

    // TODO(sean): Later we could put light as entity in world
    VLight Lights[MaxLights];
    i32 NumLights;

public:
    VRenderer() :
        RenderContext(RenderSpec)
    {}

    void StartUp(const VRenderSpecification& InRenderSpec);
    void ShutDown();

    VLN_FINLINE const VRenderSpecification& GetRenderSpec() const
    {
        return RenderSpec;
    }

    VLN_FINLINE i32 GetScreenWidth() const
    {
        return RenderSpec.TargetSize.X;
    }
    VLN_FINLINE i32 GetScreenHeight() const
    {
        return RenderSpec.TargetSize.Y;
    }

    void ResetMaterials()
    {
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

    VLN_FINLINE void PrepareToRender()
    {
        BackSurface.FillRectHW(nullptr, MAP_XRGB32(0x00, 0x00, 0x00));
        RenderContext.PrepareToRender();
    }

    VLN_FINLINE void Render()
    {
        u32* Buffer;
        i32 Pitch;
        BackSurface.Lock(Buffer, Pitch);

        RenderContext.RenderWorld(Buffer, Pitch);

        BackSurface.Unlock();
    }

    VLN_FINLINE void RenderUI()
    {
        // TODO(sean): Implement
    }

    void Flip();

    // Very slow put pixel function to debug draw functions
    void PutPixel(u32* Buffer, i32 Pitch, i32 X, i32 Y, u32 Color) const
    {
        VLN_ASSERT(X >= 0);
        VLN_ASSERT(X < RenderSpec.TargetSize.X);
        VLN_ASSERT(Y >= 0);
        VLN_ASSERT(Y < RenderSpec.TargetSize.Y);

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

    void DrawTriangle(VInterpolationContext& InterpolationContext);
    void DrawText(i32 X, i32 Y, VColorARGB Color, const char* Format, ...);

    friend class VSurface;
    friend class VMesh;
    friend class VRenderContext;
};

extern VRenderer Renderer;

}
