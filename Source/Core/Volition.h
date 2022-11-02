#ifndef CORE_VOLITION_H_

#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/DebugLog.h"
#include "Core/Window.h"
#include "Graphics/Graphics.h"
#include "Input/Input.h"
#include "Game/Game.h"

static constexpr char WindowTitle[] = "Volition";
static constexpr i32 WindowWidth = 1280;
static constexpr i32 WindowHeight = 720;
static constexpr u32 DefaultFPS = 60;

class VVolition
{
    u32 MsFrameLimit;
    u32 LastTick;
    f32 Delta;

    b32 bRunning;
public:
    void StartUp()
    {
        MsFrameLimit = 1000u / DefaultFPS;
        Delta = 0.0f;

        DebugLog.StartUp();
        Window.Create(WindowTitle, WindowWidth, WindowHeight);
        Graphics.StartUp();
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
        Graphics.ShutDown();
        Window.Destroy();
        DebugLog.ShutDown();
    }

    void Run()
    {
        while (bRunning)
        {
            TickFrame();

            HandleEvents();
            Game.Update(Delta);

            Graphics.PrepareToRender();
            Game.Render();
            Graphics.Render();

            //SyncFrame();
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

#define CORE_VOLITION_H_
#endif
