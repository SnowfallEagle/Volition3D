#ifndef GAME_GAME_H_

#include "Core/Types.h"

class VGame
{
public:
    void StartUp();
    void ShutDown();

    void Update(f32 Delta);
    void Render();
};

extern VGame Game;

#define GAME_GAME_H_
#endif
