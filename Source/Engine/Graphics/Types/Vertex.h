#pragma once

#include "Common/Platform/Platform.h"
#include "Common/Types/Common.h"
#include "Common/Math/Vector.h"

namespace Volition
{

namespace EVertexAttr
{
	enum Type
	{
		HasNormal        = VLN_BIT(1),
		HasTextureCoords = VLN_BIT(2),
	};
}

VLN_DECL_ALIGN_SSE() class VVertex
{
public:
	u32 Attr;

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

public:
    VLN_DEFINE_ALIGN_OPERATORS_SSE()
};

}
