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
		Active   = VLN_BIT(1),
		Clipped  = VLN_BIT(2),
		BackFace = VLN_BIT(3),
		Lit      = VLN_BIT(4)
	};
}

namespace EPolyAttr
{
	enum
	{
		RGB32       = VLN_BIT(1),
		TwoSided    = VLN_BIT(2),
		Transparent = VLN_BIT(3),

		ShadeModeEmissive = VLN_BIT(4),
		ShadeModeFlat     = VLN_BIT(5),
		ShadeModeGouraud  = VLN_BIT(6),
		ShadeModePhong    = VLN_BIT(7),
		ShadeModeTexture  = VLN_BIT(8),

		UsesMaterial = VLN_BIT(9)
	};
}

class VPoly
{
public:
	u32 State;
	u32 Attr;

	VColorARGB OriginalColor;
	VColorARGB LitColor[3]; // For each vertex, 0 for flat shading

	const VMaterial* Material;

	i32 VtxIndices[3];

	const VPoint2* TextureCoordsList;
	i32 TextureCoordsIndices[3];

	f32 NormalLength;
};

class VPolyFace
{
public:
	u32 State;
	u32 Attr;

	VColorARGB OriginalColor;
	VColorARGB LitColor[3];

	const VMaterial* Material;

	VVertex LocalVtx[3];
	VVertex TransVtx[3];

	f32 NormalLength;
};
