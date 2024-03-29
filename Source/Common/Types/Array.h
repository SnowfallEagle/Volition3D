#pragma once

#include <vector>
#include "Common/Types/Common.h"
#include "Common/Platform/Platform.h"

namespace Volition
{

template<typename T>
class TArray : public std::vector<T>
{
public:
    using Super = std::vector<T>;

public:
    VLN_FINLINE TArray() = default;
    VLN_FINLINE TArray(VSizeType Size) : Super(Size) {}
    VLN_FINLINE TArray(VSizeType Size, const T& Instance) : Super(Size, Instance) {}

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

    VLN_FINLINE void Remove(T& Instance)
    {
        auto _ = std::remove(Super::begin(), Super::end(), Instance);
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

    VLN_FINLINE VSizeType GetCapacity() const
    {
        return Super::capacity();
    }

    VLN_FINLINE T* GetData() const
    {
        return (T*)Super::data();
    }
};

}