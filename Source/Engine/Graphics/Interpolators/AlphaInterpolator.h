#pragma once

#include "Engine/Graphics/Interpolators/IInterpolator.h"

namespace Volition
{

class VAlphaInterpolator : public IInterpolator
{
public:
    i32 Alpha;

public:
    VAlphaInterpolator();
};

}