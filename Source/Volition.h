#ifndef VOLITION_H_

#include "Types.h"
#include "Window.h"
#include "Graphics.h"
#include "Input.h"
#include "Game.h"
#include "Platform.h"

static constexpr char WindowTitle[] = "Volition";
static constexpr i32 WindowWidth = 1280;
static constexpr i32 WindowHeight = 720;
static constexpr u32 DefaultFPS = 60;

#define GetTicks() SDL_GetTicks()

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

#define VOLITION_H_
#endif
