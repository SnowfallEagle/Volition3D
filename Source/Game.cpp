#include "Volition.h"
#include "Input.h"
#include "Game.h"

VGame Game;

void VGame::Update(f32 Delta)
{
    if (Input.IsKeyDown(SDLK_ESCAPE))
        Volition.Stop();
}

void VGame::Render()
{

}
