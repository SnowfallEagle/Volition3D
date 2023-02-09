#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Core/Containers/SparseArray.h"

class VGameFlow
{
public:
    virtual void StartUp()
    {}

    virtual void ShutDown()
    {}

    virtual void Update(f32 DeltaTime)
    {}
};