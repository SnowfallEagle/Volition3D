#pragma once

#include "Common/Types/Common.h"
#include "Common/Platform/Platform.h"
#include "Engine/Core/Config/WindowSpecification.h"

struct SDL_Window;

namespace Volition
{

class VWindow
{
    SDL_Window* SDLWindow;

public:
    void StartUp();
    void ShutDown();
    void ProcessEvents();

    friend class VRenderer;
};

inline VWindow Window;

}