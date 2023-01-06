#pragma once

#include "Core/Types.h"
#include "Math/Vector.h"
#include "Graphics/Color.h"
#include "Graphics/Vertex.h"
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

		ShadeModeEmissive = BIT(4),
		ShadeModeFlat     = BIT(5),
		ShadeModeGouraud  = BIT(6),
		ShadeModePhong    = BIT(7),
		ShadeModeTexture  = BIT(8),

		UsesMaterial = BIT(9)
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
