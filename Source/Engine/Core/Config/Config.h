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

public:
    void StartUp(i32 Args, char** Argv);
};

inline VConfig Config;

}