#pragma once

#include "Common/Types/String.h"
#include "Engine/Graphics/Scene/Mesh.h"

namespace Volition
{

class VEntity
{
public:
    VString Name = "Entity";

    VMesh* Mesh = new VMesh();

public:
    virtual ~VEntity()
    {}

    virtual void Init()
    {}

    virtual void Destroy();

    virtual void Update(f32 DeltaTime)
    {}
};

}