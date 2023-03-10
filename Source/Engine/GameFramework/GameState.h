#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Core/Containers/Array.h"

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