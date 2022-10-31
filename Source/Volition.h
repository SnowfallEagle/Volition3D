#ifndef VOLITION_H_

#include "Types.h"
#include "Window.h"
#include "Game.h"

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
        Game.StartUp();

        LastTick = GetTicks();
        bRunning = true;
    }
    void ShutDown()
    {
        bRunning = false;
        Game.ShutDown();
        Window.Destroy();
    }

    void Run()
    {
        while (bRunning)
        {
            TickFrame();

            Window.HandleEvents();
            Game.Update(Delta);
            Game.Render();

            SyncFrame();
        }
    }

    void Stop()
    {
        bRunning = false;
    }

    f32 GetDelta()
    {
        return Delta;
    }

private:
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
