#pragma once

#include <string>

class VString
{
public:
    std::string String;

public:
    VString(std::string InString = "") :
        String(InString)
    {}

    VString(const char* InString) :
        String(InString)
    {}

    operator const char*() const
    {
        return String.c_str();
    }
};
