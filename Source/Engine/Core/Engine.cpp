#include "Engine/Core/Engine.h"

namespace Volition
{

void VEngine::ShutDown()
{
    bRunning = false;

    World.ShutDown();
    Time.ShutDown();
    Input.ShutDown();
    Renderer.ShutDown();
    Math.ShutDown();
    Window.ShutDown();
    Config.ShutDown();
    DebugLog.ShutDown();
}

}