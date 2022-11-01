#include "Volition.h"
#include "Input.h"
#include "Game.h"

VGame Game;

void VGame::StartUp()
{

}

void VGame::ShutDown()
{

}

void VGame::Update(f32 Delta)
{
    if (Input.IsKeyDown(EKeyCode::Escape))
        Volition.Stop();

    printf("%.3f\n", Delta);
}

void VGame::Render()
{
    
}
