#pragma once

#include "SDL.h"
#include "Engine/Core/Types.h"
#include "Engine/Core/Platform.h"
#include "Engine/Core/DebugLog.h"
#include "Engine/Core/Window.h"
#include "Engine/Math/Math.h"
#include "Engine/Graphics/Renderer.h"
#include "Engine/Input/Input.h"
#include "Engine/Core/Time.h"
#include "Engine/Core/World.h"

namespace Volition
{

class VEngine
{
    b32 bRunning;

public:
    template<typename GameStateT>
    void StartUp(const VWindowSpecification& WindowSpec, const VRenderSpecification& RenderSpec)
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

    void ShutDown();
    void Run();

    VLN_FINLINE void Stop()
    {
        bRunning = false;
    }

private:
    void HandleEvents();
};

extern VEngine Engine;

}