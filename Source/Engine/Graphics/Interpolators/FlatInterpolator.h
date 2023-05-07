#pragma once

#include "Engine/Graphics/Interpolators/IInterpolator.h"

namespace Volition
{

class VFlatInterpolator : public IInterpolator
{
public:
    VColorARGB Color;

public:
    VFlatInterpolator();
};

}
