#include "SDL.h" // SDL_main()
#include "Core/Volition.h"

int main(int Argc, char** Argv)
{
    Volition.StartUp();
    Volition.Run();
    Volition.ShutDown();

    return 0;
}
