#pragma once

#include <xstring>
#include "Engine/Core/Platform.h"

namespace Volition
{

template<typename T = char>
class VString : public std::basic_string<T>
{
public:
    using Super = std::basic_string<T>;

public:
    VString() = default;

    VString(const VString<T>& InString) : Super(InString)
    {}

    VString(const T* InString) : Super(InString)
    {}

    VLN_FINLINE operator const T*() const
    {
        return Super::c_str();
    }
};

using VWideString = VString<wchar_t>;

}
