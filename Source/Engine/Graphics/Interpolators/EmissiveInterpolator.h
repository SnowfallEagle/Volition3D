#pragma once

#include "Engine/Graphics/Interpolators/IInterpolator.h"

namespace Volition
{

class VEmissiveInterpolator : public IInterpolator
{
public:
    VColorARGB Color;

public:
    VEmissiveInterpolator();
};

}
