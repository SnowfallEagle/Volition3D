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
    Window.ShutDown();
    Config.ShutDown();
    DebugLog.ShutDown();
}

}