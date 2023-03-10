#pragma once

#include <string>
#include "Engine/Core/Platform.h"

namespace Volition
{

class VString : std::string
{
public:
    using Super = std::string;

public:
    VString(const std::string& InString = "") : Super(InString)
    {}

    VLN_FINLINE operator const char*() const
    {
        return Super::c_str();
    }
};

}
