#ifndef GAME_H_

#include "Types.h"

class VGame
{
public:
    void StartUp() {}
    void ShutDown() {}

    void Update(f32 Delta);
    void Render();
};

extern VGame Game;

#define GAME_H_
#endif
