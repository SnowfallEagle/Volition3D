#ifndef VOLITION_H_

#include "Types.h"
#include "Window.h"

static constexpr char WindowTitle[] = "Volition";
static constexpr i32 WindowWidth = 1280;
static constexpr i32 WindowHeight = 720;

class VVolition
{
public:
    void StartUp()
    {
        Window.Create(WindowTitle, WindowWidth, WindowHeight);
    }
    void ShutDown()
    {
        Window.Destroy();
    }
    void Run()
    {
        while (!Window.ShouldClose())
            {}
    }
};

extern VVolition Volition;

#define VOLITION_H_
#endif
