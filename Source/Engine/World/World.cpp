#include "Engine/Graphics/Rendering/Renderer.h"
#include "Engine/World/World.h"

namespace Volition
{

VWorld World;

void VWorld::ShutDown()
{
    if (GameState)
    {
        GameState->ShutDown();
        delete GameState;
    }

    for (const auto Entity : Entities)
    {
        if (Entity)
        {
            Entity->Destroy();
            delete Entity;
        }
    }
    Entities.Clear();

    Terrain->Destroy();
    delete Terrain;

    Cubemap.Destroy();

    VLN_SAFE_DELETE(Camera);
}

void VWorld::Update(f32 DeltaTime)
{
    if (GameState)
    {
        GameState->Update(DeltaTime);
    }

    Terrain->Update(DeltaTime);

    for (const auto Entity : Entities)
    {
        if (Entity)
        {
            Entity->Update(DeltaTime);
        }
    }

    Camera->Dir.Y = Math.Mod(Camera->Dir.Y, 360.0f);
    if (Camera->Dir.Y < 0.0f)
    {
        Camera->Dir.Y += 360.0f;
    }
}

void VWorld::FixedUpdate(f32 FixedDeltaTime)
{
    static constexpr f32 CubemapMovementEffectSpeed = 0.00175f;
    CubemapMovementEffectAngle = Math.Mod(CubemapMovementEffectAngle + (CubemapMovementEffectSpeed * FixedDeltaTime), 360.0f);
    if (CubemapMovementEffectAngle < 0.0f)
    {
        CubemapMovementEffectAngle += 360.0f;
    }
}

void VWorld::DestroyEntity(VEntity* Entity)
{
    if (Entity)
    {
        Entity->Destroy();
        delete Entity;

        i32f Length = Entities.GetLength();
        for (i32f I = 0; I < Length; ++I)
        {
            if (Entities[I] == Entity)
            {
                Entities[I] = nullptr;
                return;
            }
        }
    }
}

void VWorld::SetCubemap(const char* Path)
{
    Cubemap.Load(Path);
}

}