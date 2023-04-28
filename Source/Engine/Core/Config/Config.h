#pragma once

#include "Engine/Core/Config/WindowSpecification.h"
#include "Engine/Core/Config/RenderSpecification.h"

namespace Volition
{

class VConfig
{
public:
    VWindowSpecification WindowSpec;
    VRenderSpecification RenderSpec;

private:
    b32 bExecutedWithLauncher = false;

public:
    void StartUp(i32 Args, char** Argv);
    void ShutDown();
};

inline VConfig Config;

}