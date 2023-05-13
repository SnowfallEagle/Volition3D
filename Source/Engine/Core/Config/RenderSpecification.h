#pragma once

#include "Common/Types/Common.h"
#include "Common/Math/Vector.h"

struct SDL_PixelFormat;

namespace Volition
{

class VRenderSpecification
{
public: /** Set by user */
    i32 TargetFPS = 60;
    i32 TargetFixedFPS = 60;
    i32 MaxMipMaps = 8;

    b8 bLimitFPS        = false;
    b8 bRenderSolid     = true;
    b8 bBackfaceRemoval = true;

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

    friend class VRenderer;
    friend class VSurface;
    friend class VCamera;
};

}
