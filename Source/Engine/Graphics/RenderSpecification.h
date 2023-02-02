#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Vector.h"

struct SDL_PixelFormat;

class VRenderSpecification
{
public: // Set by user
    i32 TargetFPS = 60;
    b32 bLimitFPS = false;

    // TODO(sean): Use z-values from here instead of from camera
    f32 ZNear = 50;
    f32 ZFar = 1200;

    f32 BestDrawDistance = 0.5f;
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

