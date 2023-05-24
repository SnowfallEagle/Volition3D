#pragma once

#include "SDL.h"
#include "SDL_ttf.h"
#include "Common/Types/Common.h"
#include "Common/Platform/Platform.h"
#include "Common/Platform/Assert.h"
#include "Common/Platform/Memory.h"
#include "Common/Math/Math.h"
#include "Common/Math/Rect.h"
#include "Common/Math/Vector.h"
#include "Common/Math/Fixed16.h"
#include "Engine/Core/Config/Config.h"
#include "Engine/Graphics/Types/Color.h"
#include "Engine/Graphics/Types/Polygon.h"
#include "Engine/Graphics/Scene/Light.h"
#include "Engine/Graphics/Scene/Camera.h"
#include "Engine/Graphics/Rendering/Surface.h"
#include "Engine/Graphics/Rendering/ZBuffer.h"
#include "Engine/Graphics/Rendering/RenderList.h"
#include "Engine/Graphics/Rendering/InterpolationContext.h"

namespace Volition
{

class VRenderList;

class VRenderer
{
public:
    static constexpr i32f MaxBaseRenderListPoly    = 524'288;
    static constexpr i32f MaxTerrainRenderListPoly = 524'288;
    static constexpr i32f MaxCachedRenderListPoly  = MaxTerrainRenderListPoly;

private:
    struct VTextElement
    {
        static constexpr i32f TextSize = 512;

        char Text[TextSize];
        SDL_Color Color;
        VVector2i Position;
    };

private:
    VSurface VideoSurface;
    VSurface BackSurface;

    VRenderList* BaseRenderList;
    VRenderList* TerrainRenderList;

    VZBuffer ZBuffer;
    VInterpolationContext InterpolationContext;

    VMaterial ShadowMaterial;

    TTF_Font* Font;
    i32 FontCharWidth; /** In pixels */
    i32 FontCharHeight;

    i32 DebugTextY;
    TArray<VTextElement> TextQueue;

public:
    void StartUp();
    void ShutDown();

    i32 GetScreenWidth() const;
    i32 GetScreenHeight() const;

    void SetTerrain(VMesh& TerrainMesh);
    void RemoveTerrain();

    void TransformLights(const VCamera& Camera);

    void PreRender();
    void Render();
    void PostProcess();
    void RenderUI();
    void PostRender();

    /** Very slow put pixel function to debug draw functions */
    void PutPixel(u32* Buffer, i32 Pitch, i32 X, i32 Y, u32 Color) const;

    static void DrawLine(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, u32 Color);
    static void DrawLineSlow(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, u32 Color);

    b32 ClipLine(i32& X1, i32& Y1, i32& X2, i32& Y2) const;
    void DrawClippedLine(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, u32 Color) const;

    void DrawText(i32 X, i32 Y, VColorARGB Color, const char* Format, ...);
    void DrawDebugText(const char* Format, ...);

    void RefreshWindowSurface();

private:
    void DrawTriangle(VInterpolationContext& InterpolationContext);
    void VarDrawText(i32 X, i32 Y, VColorARGB Color, const char* Format, std::va_list VarList); 

    void SetInterpolators();
    void RenderSolid(const VRenderList* RenderList);
    void RenderWire(const VRenderList* RenderList);

public:
    VLN_DEFINE_ALIGN_OPERATORS_SSE()

    friend class VSurface;
    friend class VMesh;
    friend class VCubemap;
    friend class VWorld;
};

inline VRenderer Renderer;

VLN_FINLINE void VRenderer::PutPixel(u32* Buffer, i32 Pitch, i32 X, i32 Y, u32 Color) const
{
    VLN_ASSERT(X >= 0);
    VLN_ASSERT(X < Config.RenderSpec.TargetSize.X);
    VLN_ASSERT(Y >= 0);
    VLN_ASSERT(Y < Config.RenderSpec.TargetSize.Y);

    Buffer[Y*Pitch + X] = Color;
}

VLN_FINLINE void VRenderer::DrawClippedLine(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, u32 Color) const
{
    if (ClipLine(X1, Y1, X2, Y2))
    {
        DrawLine(Buffer, Pitch, X1, Y1, X2, Y2, Color);
    }
}

VLN_FINLINE i32 VRenderer::GetScreenWidth() const
{
    return Config.RenderSpec.TargetSize.X;
}

VLN_FINLINE i32 VRenderer::GetScreenHeight() const
{
    return Config.RenderSpec.TargetSize.Y;
}

VLN_FINLINE void VRenderer::RemoveTerrain()
{
    TerrainRenderList->ResetList();
}

}
