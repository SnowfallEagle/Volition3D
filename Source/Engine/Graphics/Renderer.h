#pragma once

#include "SDL.h"
#include "SDL_ttf.h"
#include "Engine/Core/Types.h"
#include "Engine/Core/Platform.h"
#include "Engine/Core/Assert.h"
#include "Engine/Core/Memory.h"
#include "Engine/Core/Config/Config.h"
#include "Engine/Math/Math.h"
#include "Engine/Math/Rect.h"
#include "Engine/Math/Vector.h"
#include "Engine/Math/Fixed16.h"
#include "Engine/Graphics/Surface.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Material.h"
#include "Engine/Graphics/Light.h"
#include "Engine/Graphics/Polygon.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/ZBuffer.h"
#include "Engine/Graphics/RenderList.h"
#include "Engine/Graphics/InterpolationContext.h"

namespace Volition
{

class VRenderList;

class VRenderer
{
public:
    static constexpr i32f MaxMaterials = 256;
    static constexpr i32f MaxLights = 8;

private:
    struct VTextElement
    {
        static constexpr i32f TextSize = 512;

        char Text[TextSize];
        SDL_Color Color;
        VVector2I Position;
    };

private:
    VSurface VideoSurface;
    VSurface BackSurface;

    VRenderList* RenderList;
    VZBuffer ZBuffer;

    VInterpolationContext InterpolationContext;

    TTF_Font* Font;
    i32 FontCharWidth; // In pixels
    i32 FontCharHeight;

    i32 DebugTextY;
    TArray<VTextElement> TextQueue;

    VMaterial Materials[MaxMaterials];
    i32 NumMaterials;

    VLight* OccluderLight = nullptr;
    VLight Lights[MaxLights];
    i32 NumLights;

    /** Stuff for rendering */
    VColorARGB (*OriginalLitColors)[3]; // [MaxPoly][3]
    VColorARGB* OriginalColors;         // [MaxPoly]
    u32* OriginalAttrs;                 // [MaxPoly]

public:
    void StartUp();
    void ShutDown();

    VLN_FINLINE i32 GetScreenWidth() const
    {
        return Config.RenderSpec.TargetSize.X;
    }
    VLN_FINLINE i32 GetScreenHeight() const
    {
        return Config.RenderSpec.TargetSize.Y;
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

    VLN_FINLINE void ResetLights()
    {
        Memory.MemSetByte(Lights, 0, sizeof(Lights));
        NumLights = 0;
    }

    VLN_FINLINE void AddLight(const VLight& InLight)
    {
        Lights[NumLights] = InLight;
        ++NumLights;
    }

    VLN_FINLINE void SetOccluderLight(i32 Index)
    {
        if (Index >= 0 && Index < NumLights)
        {
            OccluderLight = &Lights[Index];
        }
    }

    void TransformLights(const VCamera& Camera)
    {
        VMatrix44 TransMat = Camera.MatCamera;
        TransMat.C30 = TransMat.C31 = TransMat.C32 = 0.0f;

        for (i32f LightIndex = 0; LightIndex < NumLights; ++LightIndex)
        {
            VMatrix44::MulVecMat(Lights[LightIndex].Pos, TransMat, Lights[LightIndex].TransPos);
            VMatrix44::MulVecMat(Lights[LightIndex].Dir, TransMat, Lights[LightIndex].TransDir);
        }
    }

    void PreRender();
    void Render();
    void RenderUI();
    void PostRender();

    // Very slow put pixel function to debug draw functions
    VLN_FINLINE void PutPixel(u32* Buffer, i32 Pitch, i32 X, i32 Y, u32 Color) const
    {
        VLN_ASSERT(X >= 0);
        VLN_ASSERT(X < Config.RenderSpec.TargetSize.X);
        VLN_ASSERT(Y >= 0);
        VLN_ASSERT(Y < Config.RenderSpec.TargetSize.Y);

        Buffer[Y*Pitch + X] = Color;
    }

    static void DrawLine(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, u32 Color);
    static void DrawLineSlow(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, u32 Color);

    b32 ClipLine(i32& X1, i32& Y1, i32& X2, i32& Y2) const;
    VLN_FINLINE void DrawClippedLine(u32* Buffer, i32 Pitch, i32 X1, i32 Y1, i32 X2, i32 Y2, u32 Color) const
    {
        if (ClipLine(X1, Y1, X2, Y2))
        {
            DrawLine(Buffer, Pitch, X1, Y1, X2, Y2, Color);
        }
    }

    void DrawText(i32 X, i32 Y, VColorARGB Color, const char* Format, ...);
    void DrawDebugText(const char* Format, ...);

private:
    void DrawTriangle(VInterpolationContext& InterpolationContext);
    void VarDrawText(i32 X, i32 Y, VColorARGB Color, const char* Format, std::va_list VarList); 

    void SetInterpolators();
    void RenderSolid();
    void RenderWire();

    friend class VSurface;
    friend class VMesh;
    friend class VCubemap;
};

extern VRenderer Renderer;

}
