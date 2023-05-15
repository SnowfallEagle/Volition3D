#pragma once

#include "Common/Types/String.h"
#include "Common/Math/Vector.h"

namespace Volition
{

namespace EWindowSpecificationFlags
{
    enum Type
    {
        Fullscreen = VLN_BIT(1),
        Borderless = VLN_BIT(2),
        Windowed   = VLN_BIT(3),
    };
}

class VWindowSpecification
{
public:
    VString Name = "Volition";
    VVector2i Size = { 640, 480 };
    u32 Flags = EWindowSpecificationFlags::Windowed;
};

}