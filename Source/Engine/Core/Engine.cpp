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

        HandleEvents();
        World.Update(Time.GetDeltaTime());

        Renderer.PrepareToRender();
        Renderer.Render();
        Renderer.RenderUI();
        Renderer.Flip();

        Time.SyncFrame();
    }

    ShutDown();
}

void VEngine::HandleEvents()
{
    Window.HandleEvents();
    Input.HandleEvents();
}

}