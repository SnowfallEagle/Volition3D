#pragma once

#include "Core/Containers/String.h"
#include "Math/Vector.h"

namespace EWindowSpecificationFlags
{
    enum Type
    {
        Fullscreen        = VL_BIT(1),
        FullscreenDesktop = VL_BIT(2),
        Windowed          = VL_BIT(3),

        Resizable = VL_BIT(4)
    };
}

class VWindowSpecification
{
public:
    VString Name = "Volition";
    VString IconPath = "";
    VVector2I Size = { 1280, 720 };
    u32 Flags = EWindowSpecificationFlags::Windowed | EWindowSpecificationFlags::Resizable;
};