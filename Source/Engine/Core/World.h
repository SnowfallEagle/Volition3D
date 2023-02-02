#pragma once

#include "Engine/Core/Containers/SparseArray.h"
#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/GameFlow.h"

class VWorld
{
    TSparseArray<VMesh*> Meshes;
    VCamera* Camera;

    VGameFlow* GameFlow;

public:
    template <typename GAME_FLOW_TYPE>
    void StartUp()
    {
        Camera = new VCamera();

        GameFlow = new GAME_FLOW_TYPE();
        GameFlow->StartUp();
    }

    void ShutDown()
    {
        GameFlow->ShutDown();
        delete GameFlow;

        // TODO(sean): Delete entities
        for (auto Mesh : Meshes)
        {
            Mesh->Destroy();
            delete Mesh;
        }

        delete Camera;
    }

    void Update(f32 DeltaTime)
    {
        GameFlow->Update(DeltaTime);

        // TODO(sean): Update entities
    }

    VL_FINLINE VCamera* GetCamera()
    {
        return Camera;
    }

    template <typename T>
    T* SpawnMesh()
    {
        VMesh* Mesh = new T();
        // TODO(sean): Init
        Meshes.EmplaceBack(Mesh);
        return Mesh;
    }

    friend class VRenderer;
    friend class VRenderContext;
};

extern VWorld World;