#include "Engine/Core/Engine.h"

namespace Volition
{

VEngine Engine;

void VEngine::ShutDown()
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

void VEngine::Run()
{
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
}

}