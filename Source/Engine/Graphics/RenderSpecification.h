#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Vector.h"

struct SDL_PixelFormat;

class VRenderSpecification
{
public: // Set by user
    i32 TargetFPS = 60;
    b32 bLimitFPS = false;

    b32 bRenderSolid = true;
    b32 bBackfaceRemoval = true;

    f32 BestDrawDistanceFactor = 0.5f; // Factor to ZFar
    i32 MaxMipMappingLevel = 4;

private: // Set by rendering engine
    SDL_PixelFormat* SDLPixelFormat = nullptr;
    u32 SDLPixelFormatEnum = 0;

    VVector2I TargetSize = { 0, 0 };

    VVector2I MinClip = { 0, 0 };
    VVector2I MaxClip = { 0, 0 };

    VVector2 MinClipFloat = { 0.0f, 0.0f };
    VVector2 MaxClipFloat = { 0.0f, 0.0f };

    friend class VRenderer;
    friend class VRenderContext;
    friend class VSurface;
};

