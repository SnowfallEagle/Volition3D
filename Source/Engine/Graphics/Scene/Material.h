#pragma once

#include "Common/Platform/Platform.h"
#include "Common/Types/Common.h"
#include "Engine/Graphics/Types/Color.h"
#include "Engine/Graphics/Rendering/Texture.h"

namespace Volition
{

namespace EMaterialAttr
{
	enum
	{
		TwoSided    = VLN_BIT(1),
		Transparent = VLN_BIT(2),
		Terrain     = VLN_BIT(3),

		ShadeModeEmissive = VLN_BIT(4),
		ShadeModeFlat     = VLN_BIT(5),
		ShadeModeGouraud  = VLN_BIT(6),
		ShadeModeTexture  = VLN_BIT(7),
	};
}

class VMaterial
{
public:
    u32 Attr;

    VColorARGB Color;
    f32 KAmbient, KDiffuse, Power;
    VColorARGB RAmbient, RDiffuse; /** K*Color */

    VTexture Texture;

public:
    void Init();
    void Destroy();

    void ComputeReflectiveColors();
};

}
