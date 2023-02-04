#include "Engine/Core/Types.h"
#include "Engine/Math/Fixed28.h"

class VRenderSpecification;

class VInterpolationContext
{
public:
    const VRenderSpecification& RenderSpec;

    // RenderContext sets this up every frame
    u32* Buffer;
    i32 BufferPitch;

    // InterpolationContext sets this up every polygon
    u32* TextureBuffer;
    i32 TextureBufferPitch;

    // InterpolationContext sets this up during rasterization
    i32 X, Y;
    fx28 Z;

public:
    VInterpolationContext(const VRenderSpecification& InRenderSpec) :
        RenderSpec(InRenderSpec)
    {}

    void StartInterpolation()
    {
        // TODO(sean): Renderer calls when InterpolationContext should compute stuff
    }
};