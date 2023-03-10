#pragma once

#include "Engine/Core/Containers/String.h"
#include "Engine/Graphics/Mesh.h"

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

    virtual void Destroy()
    {
        if (Mesh)
        {
            Mesh->Destroy();
            delete Mesh;
            Mesh = nullptr;
        }
    }

    virtual void Update(f32 DeltaTime)
    {}
};

}