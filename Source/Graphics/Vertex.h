#pragma once

#include "Core/Platform.h"
#include "Core/Types.h"
#include "Math/Vector.h"

namespace EVertexAttr
{
	enum
	{
		HasPoint = BIT(1),
		HasNormal = BIT(2),
		HasTexture = BIT(3),
	};
}

// NOTE(sean): We count Attributes and Intensity as Components of Vertex
class Vertex
{
public:
	u32 Attr;
	f32 Intensity;

	union
	{
		f32 C[10];

		struct
		{
			VVector4 Position;
			VVector4 Normal;
			VPoint2 TextureCoords;
		};

		struct
		{
			f32 X, Y, Z, W;
			f32 NX, NY, NZ, NW;
			f32 U, V;
		};
	};
};

