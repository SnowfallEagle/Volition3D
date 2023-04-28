#pragma once

#include "Engine/Core/Types/Common.h"
#include "Engine/Core/Types/Array.h"

namespace Volition
{

class VGameState
{
public:
    virtual void StartUp()
    {}

    virtual void ShutDown()
    {}

    virtual void Update(f32 DeltaTime)
    {}
};

}