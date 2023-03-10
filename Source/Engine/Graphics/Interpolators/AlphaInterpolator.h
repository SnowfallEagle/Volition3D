#pragma once

#include "Engine/Graphics/Interpolators/IInterpolator.h"

namespace Volition
{

class VAlphaInterpolator final : public IInterpolator
{
private:
    i32 Alpha;

public:
    virtual void Start() override;
    virtual void ProcessPixel() override;
};

}