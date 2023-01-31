#pragma once

#include "Core/Platform.h"
#include "Core/Types.h"
#include "Math/Vector.h"

namespace EVertexAttr
{
	enum
	{
		HasNormal        = VL_BIT(1),
		HasTextureCoords = VL_BIT(2),
	};
}

class VVertex
{
public:
	u32 Attr;
	f32 Intensity; // TODO(sean)

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

