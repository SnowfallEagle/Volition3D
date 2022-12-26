#pragma once

#include "Core/Types.h"
#include "Math/Vector.h"
#include "Graphics/Color.h"
#include "Graphics/Surface.h"
#include "Graphics/Material.h"

namespace EPolyState
{
    enum
    {
        Active   = BIT(1),
        Clipped  = BIT(2),
        BackFace = BIT(3),
        Lit      = BIT(4)
    };
}

namespace EPolyAttr
{
    enum
    {
        RGB32       = BIT(1),
        TwoSided    = BIT(2),
        Transparent = BIT(3),

        ShadeModePure     = BIT(4),
        ShadeModeConstant = ShadeModePure,
        ShadeModeFlat     = BIT(5),
        ShadeModeGouraud  = BIT(6),
        ShadeModePhong    = BIT(7),
        ShadeModeTexture  = BIT(8),

        UseMaterial = BIT(9)
    };
}

class VPoly
{
public:
    u32 State;
    u32 Attr;

    VColorARGB OriginalColor;
    // TODO(sean)
    VColorARGB LitColor[3]; // For each vertex, 0 for flat shading

    VSurface* Texture; // TODO(sean)
    VMaterial* Material; // TODO(sean)

    VPoint4* VtxList; // TODO(sean)
    i32 Vtx[3];

    VPoint2I* TextureCoordsList; // TODO(sean)
    i32 TextureCoordsIndices[3]; // TODO(sean)

    f32 NormalLength; // TODO(sean)
};

class VPolyFace
{
public:
    u32 State;
    u32 Attr;

    VColorARGB OriginalColor; // TODO(sean)
    VColorARGB LitColor[3]; // For each vertex, 0 for flat shading

    VSurface* Texture;
    VMaterial* Material;

    VVector4 Normal;
    f32 NormalLength;

    f32 AverageZ;

    VPoint4 LocalVtx[3];
    VPoint4 TransVtx[3];
};
