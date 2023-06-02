#pragma once

#include "Common/Types/Common.h"
#include "Common/Math/Vector.h"
#include "Engine/Graphics/Types/Color.h"
#include "Engine/Graphics/Types/Vertex.h"
#include "Engine/Graphics/Rendering/Surface.h"
#include "Engine/Graphics/Scene/Material.h"

namespace Volition
{

namespace EPolyState
{
    enum Type
    {
        Active   = VLN_BIT(1),
        Clipped  = VLN_BIT(2),
        Backface = VLN_BIT(3),
        Lit      = VLN_BIT(4),

        NotRenderTest = Clipped | Backface,
        NotLightTest  = NotRenderTest | Lit,
    };
}

class VPoly
{
public:
    u32 State;

    const VMaterial* Material;

    i32 VtxIndices[3];
    i32 TextureCoordsIndices[3];

    VColorARGB LitColor[3]; /** For each vertex, 0 for flat shading */
    f32 NormalLength;
};

class VPolyFace
{
public:
    u32 State;

    const VMaterial* Material;

    VVertex LocalVtx[3];
    VVertex TransVtx[3];

    VColorARGB LitColor[3];
    f32 NormalLength;
};

}
