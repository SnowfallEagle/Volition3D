#pragma once

#include "Engine/Core/Types.h"

class VGame
{
public:
    void StartUp();
    void ShutDown();

    void Update(f32 Delta);
    void Render();
};

extern VGame Game;
