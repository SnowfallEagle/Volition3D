#pragma once

#include "SDL.h"
#include "Engine/Core/Types/Common.h"
#include "Engine/Core/Platform/Platform.h"
#include "Engine/Core/DebugLog.h"
#include "Engine/Core/Window.h"
#include "Engine/Core/Time.h"
#include "Engine/World/World.h"
#include "Engine/Input/Input.h"
#include "Engine/Math/Math.h"
#include "Engine/Graphics/Rendering/Renderer.h"

namespace Volition
{

class VEngine
{
    b32 bRunning;

public:
    template<typename GameStateT>
    i32 Run(i32 Argc, char** Argv);

    void Stop();

private:
    template<typename GameStateT>
    void StartUp(i32 Argc, char** Argv);

    void ShutDown();
};

extern VEngine Engine;

template<typename GameStateT>
inline void VEngine::StartUp(i32 Argc, char** Argv)
{
    DebugLog.StartUp();
    Config.StartUp(Argc, Argv);
    Window.StartUp();
    Math.StartUp();
    Renderer.StartUp();
    Input.StartUp();
    Time.StartUp();
    World.StartUp<GameStateT>();

    bRunning = true;
}

template<typename GameStateT>
inline i32 VEngine::Run(i32 Argc, char** Argv)
{
    StartUp<GameStateT>(Argc, Argv);

    while (bRunning)
    {
        Time.TickFrame();

        Window.ProcessEvents();
        Input.ProcessEvents();

        World.Update(Time.GetDeltaTime());

        i32f FixedUpdates = Time.GetNumFixedUpdates();
        for (i32f I = 0; I < FixedUpdates; ++I)
        {
            World.FixedUpdate(Time.GetFixedDeltaTime());
        }

        Renderer.PreRender();
        Renderer.Render();
        Renderer.RenderUI();
        Renderer.PostRender();

        Time.SyncFrame();
    }

    ShutDown();
    return 0;
}

inline void VEngine::Stop()
{
    bRunning = false;
}

}