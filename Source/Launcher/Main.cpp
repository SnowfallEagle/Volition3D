#include "Launcher/Launcher.h"

using namespace Volition;

namespace Game
{

class GLauncher : public VLauncher
{
    virtual void Update()
    {
    }
};

}

int main(int Argc, char** Argv)
{
    Game::GLauncher Launcher;
    return Launcher.Run();
}
