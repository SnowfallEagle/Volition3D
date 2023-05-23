#pragma once

#include "Common/Types/Common.h"
#include "Common/Math/Vector.h"

struct SDL_PixelFormat;

namespace Volition
{

class VRenderSpecification
{
public:
    static constexpr VVector3 DefaultColorCorrection = { 1.0f, 1.0f, 1.0f };

public: /** Set by user */
    b32 bLimitFPS        : 1;
    b32 bRenderSolid     : 1;
    b32 bBackfaceRemoval : 1;
    b32 bPostProcessing  : 1;
    b32 bRenderUI        : 1;

    i32 TargetFPS = 60;
    i32 TargetFixedFPS = 60;
    i32 MaxMipMaps = 8;

    VVector3 PostProcessColorCorrection = DefaultColorCorrection;

private: /** Set by rendering engine */
    SDL_PixelFormat* SDLPixelFormat = nullptr;
    u32 SDLPixelFormatEnum = 0;

    i32 BitsPerPixel = 32;
    i32 BytesPerPixel = 4;

    VVector2i TargetSize = { 0, 0 };

    VVector2i MinClip = { 0, 0 };
    VVector2i MaxClip = { 0, 0 };

    VVector2 MinClipFloat = { 0.0f, 0.0f };
    VVector2 MaxClipFloat = { 0.0f, 0.0f };

public:
    VRenderSpecification()
    {
        bLimitFPS        = false;
        bRenderSolid     = true;
        bBackfaceRemoval = true;
        bPostProcessing  = true;
        bRenderUI        = true;
    }

    friend class VRenderer;
    friend class VSurface;
    friend class VCamera;
};

}
