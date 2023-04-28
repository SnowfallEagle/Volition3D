#pragma once

#include <vector>
#include "Engine/Core/Types/Common.h"
#include "Engine/Core/Platform/Platform.h"

namespace Volition
{

template<typename T>
class TArray : public std::vector<T>
{
public:
    using Super = std::vector<T>;

public:
    VLN_FINLINE TArray() = default;

    VLN_FINLINE TArray(VSizeType Size) : Super(Size)
    {}

    VLN_FINLINE TArray(VSizeType Size, const T& Instance) : Super(Size, Instance)
    {}

    VLN_FINLINE void Resize(VSizeType Size)
    {
        Super::resize(Size);
    }

    VLN_FINLINE void Reserve(VSizeType Size)
    {
        Super::reserve(Size);
    }

    VLN_FINLINE void Erase(VSizeType Index)
    {
        Super::erase(Index);
    }

    VLN_FINLINE void Clear()
    {
        Super::clear();
    }

    template<typename... ArgsT>
    VLN_FINLINE T& EmplaceBack(ArgsT&&... Args)
    {
        Super::emplace_back(std::forward<ArgsT>(Args)...);
        return GetBack();
    }

    VLN_FINLINE T& GetBack()
    {
        return Super::back();
    }

    VLN_FINLINE VSizeType GetLength() const
    {
        return Super::size();
    }
};

}