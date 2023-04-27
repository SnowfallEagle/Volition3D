#pragma once

#include "SDL.h"
#include "Engine/Core/Types.h"
#include "Engine/Core/Platform.h"
#include "Engine/Core/DebugLog.h"
#include "Engine/Core/Window.h"
#include "Engine/Core/Time.h"
#include "Engine/Core/World.h"
#include "Engine/Input/Input.h"
#include "Engine/Math/Math.h"
#include "Engine/Graphics/Renderer.h"

namespace Volition
{

class VEngine
{
    b32 bRunning;

public:
    template<typename GameStateT>
    i32 Run(const VWindowSpecification& WindowSpec, const VRenderSpecification& RenderSpec);

    void Stop();

private:
    template<typename GameStateT>
    void StartUp(const VWindowSpecification& WindowSpec, const VRenderSpecification& RenderSpec);

    void ShutDown();
};

extern VEngine Engine;

template<typename GameStateT>
inline void VEngine::StartUp(const VWindowSpecification& WindowSpec, const VRenderSpecification& RenderSpec)
{
    DebugLog.StartUp();
    Window.Create(WindowSpec);
    Math.StartUp();
    Renderer.StartUp(RenderSpec);
    Input.StartUp();
    Time.StartUp(RenderSpec);
    World.StartUp<GameStateT>();

    bRunning = true;
}

template<typename GameStateT>
inline i32 VEngine::Run(const VWindowSpecification& WindowSpec, const VRenderSpecification& RenderSpec)
{
    StartUp<GameStateT>(WindowSpec, RenderSpec);

    while (bRunning)
    {
        Time.TickFrame();

        Window.HandleEvents();
        Input.HandleEvents();

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