#pragma once

#include <string>

class VString : std::string
{
public:
    using Super = std::string;

public:
    VString(const std::string& InString = "") : Super(InString)
    {}

    operator const char*() const
    {
        return Super::c_str();
    }
};
