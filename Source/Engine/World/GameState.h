#pragma once

#include "Common/Types/Common.h"
#include "Common/Types/Array.h"

namespace Volition
{

class VGameState
{
public:
    virtual void StartUp() {}
    virtual void ShutDown() {}
    virtual void Update(f32 DeltaTime) {}
};

}