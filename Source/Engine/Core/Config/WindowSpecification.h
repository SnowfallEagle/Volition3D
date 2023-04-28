#pragma once

#include "Engine/Core/Types/String.h"
#include "Engine/Math/Vector.h"

namespace Volition
{

namespace EWindowSpecificationFlags
{
    enum Type
    {
        Fullscreen        = VLN_BIT(1),
        FullscreenDesktop = VLN_BIT(2),
        Windowed          = VLN_BIT(3),
    };
}

class VWindowSpecification
{
public:
    VString Name = "Volition";
    VString IconPath = "";
    VVector2i Size = { 1280, 720 };
    u32 Flags = EWindowSpecificationFlags::Windowed;
};

}