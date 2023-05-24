#pragma once

#include <xstring>
#include "Common/Platform/Platform.h"

namespace Volition
{

template<typename T>
class VStringBase : public std::basic_string<T>
{
public:
    using Super = std::basic_string<T>;

public:
    VStringBase() = default;
    VStringBase(const VStringBase<T>& InString) : Super(InString) {}
    VStringBase(const T* InString) : Super(InString) {}

    VLN_FINLINE operator const T*() const
    {
        return Super::c_str();
    }

    VLN_FINLINE const T* operator *() const
    {
        return Super::c_str();
    }
};

using VString = VStringBase<char>;
using VWideString = VStringBase<wchar_t>;

}
