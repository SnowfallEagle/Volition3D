#include "Core/Volition.h"
#include "Input/Input.h"
#include "Game/Game.h"

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
