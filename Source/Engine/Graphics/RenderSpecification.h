#pragma once

#include "Engine/Core/Types.h"

class VRenderSpecification
{
public:
    i32 TargetFPS = 60;
    b32 bLimitFPS = false;

    // TODO(sean): Use z-values from here instead of from camera
    f32 ZNear = 50;
    f32 ZFar = 1200;

    f32 BestDrawDistance = 0.5f;
    i32 MaxMipMappingLevel = 4;
};

