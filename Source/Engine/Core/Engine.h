#pragma once

#include "Common/Types/Common.h"
#include "Common/Platform/Platform.h"
#include "Common/Math/Math.h"
#include "Engine/Core/DebugLog.h"
#include "Engine/Core/Window.h"
#include "Engine/Core/Time.h"
#include "Engine/Core/Events/EventBus.h"
#include "Engine/World/World.h"
#include "Engine/Input/Input.h"
#include "Engine/Graphics/Rendering/Renderer.h"

namespace Volition
{

class VEngine
{
    b32 bRunning;

public:
    template<typename GameStateT = VGameState>
    i32 Run(i32 Argc, char** Argv);
    void Stop();

private:
    template<typename GameStateT = VGameState>
    void StartUp(i32 Argc, char** Argv);
    void ShutDown();
};

inline VEngine Engine;

template<typename GameStateT>
void VEngine::StartUp(i32 Argc, char** Argv)
{
    DebugLog.StartUp();
    Config.StartUp(Argc, Argv);
    EventBus.StartUp();
    Window.StartUp();
    Math.StartUp();
    Renderer.StartUp();
    Input.StartUp();
    Time.StartUp();
    World.StartUp<GameStateT>();

    bRunning = true;
}

template<typename GameStateT>
i32 VEngine::Run(i32 Argc, char** Argv)
{
    StartUp<GameStateT>(Argc, Argv);

    while (bRunning)
    {
        // Get delta time
        Time.TickFrame();

        // Process all events
        EventBus.Update();
        Window.ProcessEvents();
        Input.ProcessEvents();

        // Update world
        World.Update(Time.GetDeltaTime());

        i32f FixedUpdates = Time.GetNumFixedUpdates();
        for (i32f i = 0; i < FixedUpdates; ++i)
        {
            World.FixedUpdate(Time.GetFixedDeltaTime());
        }

        // Render frame
        Renderer.PreRender();
        Renderer.Render();
        Renderer.PostProcess();
        Renderer.RenderUI();
        Renderer.PostRender();

        // Synchronize fps
        Time.SyncFrame();
    }

    ShutDown();
    return 0;
}

VLN_FINLINE void VEngine::Stop()
{
    bRunning = false;
}

}