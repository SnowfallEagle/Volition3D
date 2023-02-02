#pragma once

#include "Engine/Core/Types.h"

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