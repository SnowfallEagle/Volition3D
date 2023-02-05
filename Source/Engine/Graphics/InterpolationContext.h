#include "Engine/Core/Types.h"
#include "Engine/Math/Fixed28.h"

class VRenderSpecification;

class VInterpolationContext
{
// TODO(sean): Of course it would be better to use getters, setters to prevent Renderer to set value in TextureBuffer...
public:
    const VRenderSpecification& RenderSpec;

    // RenderContext sets this up every frame
    u32* Buffer;
    i32 BufferPitch;

    // InterpolationContext sets this up every polygon face
    VPolyFace* Poly; // TODO(sean): Remove it, use only it's properties such as material, Origin/Lit Colors
    i32 VtxIndices[3];

    u32* TextureBuffer;
    i32 TextureBufferPitch;

    // InterpolationContext sets this up during rasterization
    VColorARGB Pixel;
    i32f X, Y;
    fx28 Z;

public:
    VInterpolationContext(const VRenderSpecification& InRenderSpec) :
        RenderSpec(InRenderSpec)
    {}

    void Start()
    {
        // TODO(sean): Renderer calls when InterpolationContext should choose texture, set interpolators and so on
    }

    void End()
    {
        // TODO(sean): Unlock texture...
    }

    // TODO(sean): Swap VtxIndices here...
};