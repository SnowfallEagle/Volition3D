#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Vector.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Vertex.h"
#include "Engine/Graphics/Surface.h"
#include "Engine/Graphics/Material.h"

namespace EPolyState
{
	enum
	{
		Active   = VL_BIT(1),
		Clipped  = VL_BIT(2),
		BackFace = VL_BIT(3),
		Lit      = VL_BIT(4)
	};
}

namespace EPolyAttr
{
	enum
	{
		RGB32       = VL_BIT(1),
		TwoSided    = VL_BIT(2),
		Transparent = VL_BIT(3),

		ShadeModeEmissive = VL_BIT(4),
		ShadeModeFlat     = VL_BIT(5),
		ShadeModeGouraud  = VL_BIT(6),
		ShadeModePhong    = VL_BIT(7),
		ShadeModeTexture  = VL_BIT(8),

		UsesMaterial = VL_BIT(9)
	};
}

class VPoly
{
public:
	u32 State;
	u32 Attr;

	VColorARGB OriginalColor;
	VColorARGB LitColor[3]; // For each vertex, 0 for flat shading

	VSurface* Texture;
	VMaterial* Material;

	i32 VtxIndices[3];

	VPoint2* TextureCoordsList;
	i32 TextureCoordsIndices[3];

	f32 NormalLength;
};

class VPolyFace
{
public:
	u32 State;
	u32 Attr;

	VColorARGB OriginalColor;
	VColorARGB LitColor[3]; // For each vertex, 0 for flat shading

	VSurface* Texture;
	VMaterial* Material;

	VVertex LocalVtx[3];
	VVertex TransVtx[3];

	VVector4 Normal; // TODO(sean)
	f32 NormalLength;

	f32 AverageZ; // TODO(sean)
};
