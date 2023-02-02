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
    SDL_PixelFormat* SDLPixelFormat;
    u32 SDLPixelFormatEnum;

    VVector2I TargetSize;

    VVector2I MinClip;
    VVector2I MaxClip;

    VVector2 MinClipFloat;
    VVector2 MaxClipFloat;

    friend class VRenderer;
    friend class VRenderContext;
    friend class VSurface;
};

