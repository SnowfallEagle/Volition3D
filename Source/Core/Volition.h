#pragma once

#include <cstdlib>
#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/DebugLog.h"
#include "Core/Window.h"
#include "Math/Math.h"
#include "Graphics/Renderer.h"
#include "Input/Input.h"
#include "GameFramework/Game.h"

class VVolition
{
    u32 MsFrameLimit;
    u32 LastTick;
    f32 Delta;

    b32 bRunning;

public:
    void StartUp(const VWindowSpecification& WindowSpec, const VRenderSpecification& RenderSpec)
    {
        MsFrameLimit = 1000u / RenderSpec.TargetFPS;
        Delta = 0.0f;

        DebugLog.StartUp();
        Window.Create(WindowSpec);
        Math.StartUp();
        Renderer.StartUp();
        Input.StartUp();
        Game.StartUp();

        LastTick = GetTicks();
        bRunning = true;
    }
    void ShutDown()
    {
        bRunning = false;

        Game.ShutDown();
        Input.ShutDown();
        Renderer.ShutDown();
        Math.ShutDown();
        Window.Destroy();
        DebugLog.ShutDown();
    }

    void Run()
    {
        while (bRunning)
        {
            // TODO(sean): Time.TickFrame()
            TickFrame();

            HandleEvents();
            Game.Update(Delta);

            Renderer.PrepareToRender();
            Game.Render();
            Renderer.RenderAndFlip();

            // TODO(sean): Time.SyncFrame()
        }
    }
    FINLINE void Stop()
    {
        bRunning = false;
    }

    FINLINE u32 GetTicks()
    {
        return SDL_GetTicks();
    }
    FINLINE f32 GetDelta()
    {
        return Delta;
    }

private:
    void HandleEvents()
    {
        Window.HandleEvents();
        Input.HandleEvents();
    }

    void TickFrame()
    {
        u32 CurrentTick = GetTicks();
        Delta = (f32)(CurrentTick - LastTick);
        LastTick = CurrentTick;
    }
    void SyncFrame()
    {
        while (GetTicks() - LastTick < MsFrameLimit)
           {}
    }
};

extern VVolition Volition;
