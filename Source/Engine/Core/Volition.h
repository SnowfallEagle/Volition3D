#pragma once

#include <cstdlib>
#include "SDL.h"
#include "Engine/Core/Types.h"
#include "Engine/Core/Platform.h"
#include "Engine/Core/DebugLog.h"
#include "Engine/Core/Window.h"
#include "Engine/Math/Math.h"
#include "Engine/Graphics/Renderer.h"
#include "Engine/Input/Input.h"
#include "Engine/Core/Time.h"
#include "Engine/GameFramework/Game.h"

class VVolition
{
    b32 bRunning;

public:
    void StartUp(const VWindowSpecification& WindowSpec, const VRenderSpecification& RenderSpec)
    {
        DebugLog.StartUp();
        Window.Create(WindowSpec);
        Math.StartUp();
        Renderer.StartUp();
        Input.StartUp();
        Time.StartUp(RenderSpec);
        Game.StartUp();

        bRunning = true;
    }

    void ShutDown()
    {
        bRunning = false;

        Game.ShutDown();
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
            Game.Update(Time.GetDeltaTime());

            Renderer.PrepareToRender();
            Game.Render();
            Renderer.RenderAndFlip();

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
