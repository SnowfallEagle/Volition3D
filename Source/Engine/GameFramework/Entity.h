#pragma once

#include "Engine/Core/Containers/String.h"
#include "Engine/Graphics/Mesh.h"

class VEntity
{
public:
    VString Name = "Entity";

    VMesh* Mesh = nullptr;
    VMatrix44 MatTransform = VMatrix44::Identity; // TODO(sean): Implement

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