#ifndef VOLITION_H_

#include "Types.h"
#include "Window.h"

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

        Window.Create(WindowTitle, WindowWidth, WindowHeight);

        LastTick = SDL_GetTicks();
        bRunning = true;
    }
    void ShutDown()
    {
        bRunning = false;
        Window.Destroy();
    }

    void Run()
    {
        while (bRunning)
        {
            TickFrame();

            HandleEvents();

            SyncFrame();
        }
    }

private:
    void TickFrame()
    {
        u32 CurrentTick = SDL_GetTicks();
        Delta = (f32)(CurrentTick - LastTick);
        LastTick = CurrentTick;
    }

    void SyncFrame()
    {
        while (SDL_GetTicks() - LastTick < MsFrameLimit)
            {}
    }

    void HandleEvents()
    {
        SDL_Event Event;

        while (SDL_PollEvent(&Event))
        {
            switch (Event.type)
            {
            case SDL_QUIT:
            {
                bRunning = false;
            } break;

            default: {} break;
            }
        }
    }
};

extern VVolition Volition;

#define VOLITION_H_
#endif
