#pragma once

#include <vector>
#include "Engine/Core/Types.h"
#include "Engine/Core/Platform.h"

template <typename T>
class TArray : public std::vector<T>
{
public:
    using Super = std::vector<T>;

public:
    VL_FINLINE TArray() = default;

    VL_FINLINE TArray(VSizeType Size) : Super(Size)
    {}

    VL_FINLINE TArray(VSizeType Size, const T& Instance) : Super(Size, Instance)
    {}

    VL_FINLINE void Resize(VSizeType Size)
    {
        Super::resize(Size);
    }

    template<typename... ArgsT>
    VL_FINLINE T& EmplaceBack(ArgsT&&... Args)
    {
        Super::emplace_back(std::forward<ArgsT>(Args)...);
        return GetBack();
    }

    VL_FINLINE T& GetBack()
    {
        return Super::back();
    }

    VL_FINLINE VSizeType GetLength() const
    {
        return Super::size();
    }
};
