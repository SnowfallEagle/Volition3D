#pragma once

#include <cstdlib>
#include "SDL.h"
#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/DebugLog.h"
#include "Core/Window.h"
#include "Math/Math.h"
#include "Graphics/Renderer.h"
#include "Input/Input.h"
#include "Core/Time.h"
#include "GameFramework/Game.h"

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

    FINLINE void Stop()
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
