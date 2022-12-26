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
	VColorARGB LitColor[3]; // For each vertex, 0 for flat shading

	VSurface* Texture;
	VMaterial* Material;

	// TODO(sean)
	union
	{
		VVertex* _VtxList;
		VPoint4* VtxList;
	};
	i32 Vtx[3];

	VPoint2* TextureCoordsList;
	i32 TextureCoordsIndices[3];

	f32 NormalLength;
};

class VPolyFace
{
public:
	u32 State;
	u32 Attr;

	VColorARGB OriginalColor; // FIXME(sean): Do we need this field?
	VColorARGB LitColor[3]; // For each vertex, 0 for flat shading

	VSurface* Texture;
	VMaterial* Material;

	VVector4 Normal;
	f32 NormalLength;

	f32 AverageZ;

	// TODO(sean)
	union
	{
		struct
		{
			VVertex _LocalVtx[3];
			VVertex _TransVtx[3];
		};
		struct
		{
			VPoint4 LocalVtx[3];
			VPoint4 TransVtx[3];
		};
	};
};
