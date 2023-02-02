#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Core/Containers/SparseArray.h"
#include "Engine/GameFramework/GameState.h"

class VGameFlow
{
    TSparseArray<VGameState*> GameStates;

public:
    virtual void StartUp()
    {}

    virtual void ShutDown()
    {
        // TODO(sean): Delete game states
    }

    virtual void Update(f32 DeltaTime)
    {
        /*
        if (GameStates.GetBack())
        {
            GameStates.GetBack()->Update(DeltaTime);
        }
        */
    }

    // TODO(sean): Implement game state machine here
};