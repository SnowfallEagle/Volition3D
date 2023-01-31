#include "Core/Volition.h"

int main(int Argc, char** Argv)
{
    VWindowSpecification WindowSpec;
    VRenderSpecification RenderSpec;

    Volition.StartUp(WindowSpec, RenderSpec);
    Volition.Run();

    return 0;
}