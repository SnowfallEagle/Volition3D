#pragma once

#include "Engine/Graphics/Interpolators/IInterpolator.h"

namespace Volition
{

class VFlatInterpolator final : public IInterpolator
{
private:
    VColorARGB Color;

public:
    virtual ~VFlatInterpolator() = default;

    virtual void Start() override;
    virtual void ProcessPixel() override;
};

}
