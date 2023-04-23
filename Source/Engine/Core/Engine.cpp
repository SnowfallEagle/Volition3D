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

        Renderer.PreRender();
        Renderer.Render();
        Renderer.RenderUI();
        Renderer.PostRender();

        Time.SyncFrame();
    }

    ShutDown();
}

}