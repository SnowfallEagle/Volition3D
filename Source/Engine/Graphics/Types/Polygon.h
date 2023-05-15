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
	enum
	{
		Active   = VLN_BIT(1),
		Clipped  = VLN_BIT(2),
		Backface = VLN_BIT(3),
		Lit      = VLN_BIT(4),

        NotLightTest = Clipped | Backface | Lit
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

		UsesMaterial = VLN_BIT(9),
		Terrain      = VLN_BIT(10),
	};
}

class VPoly
{
public:
	u32 State;
	u32 Attr;                 // @TODO: Move in Material

	VColorARGB OriginalColor; // @TODO: Move in material
	VColorARGB LitColor[3]; /** For each vertex, 0 for flat shading */

	const VMaterial* Material;

	i32 VtxIndices[3];
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

}
