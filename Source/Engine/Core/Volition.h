#pragma once

#include "SDL.h"
#include "Engine/Core/Types.h"
#include "Engine/Core/Platform.h"
#include "Engine/Core/DebugLog.h"
#include "Engine/Core/Window.h"
#include "Engine/Math/Math.h"
#include "Engine/Graphics/Renderer.h"
#include "Engine/Input/Input.h"
#include "Engine/Core/Time.h"
#include "Engine/Core/World.h"

class VVolition
{
    b32 bRunning;

public:
    template <typename GAME_WORLD_TYPE>
    void StartUp(const VWindowSpecification& WindowSpec, const VRenderSpecification& RenderSpec)
    {
        DebugLog.StartUp();
        Window.Create(WindowSpec);
        Math.StartUp();
        Renderer.StartUp(RenderSpec);
        Input.StartUp();
        Time.StartUp(RenderSpec);
        World.StartUp<GAME_WORLD_TYPE>();

        bRunning = true;
    }

    void ShutDown()
    {
        bRunning = false;

        World.ShutDown();
        Time.ShutDown();
        Input.ShutDown();
        Renderer.ShutDown();
        Math.ShutDown();
        Window.Destroy();
        DebugLog.ShutDown();
    }

    void Run()
    {
        // Initial tick
        Time.TickFrame();

        while (bRunning)
        {
            Time.TickFrame();

            HandleEvents();
            World.Update(Time.GetDeltaTime());

            Renderer.PrepareToRender();
            Renderer.RenderWorld();
            Renderer.Flip();

            Time.SyncFrame();
        }

        ShutDown();
    }

    VL_FINLINE void Stop()
    {
        bRunning = false;
    }

private:
    void HandleEvents()
    {
        Window.HandleEvents();
        Input.HandleEvents();
    }
};

extern VVolition Volition;
