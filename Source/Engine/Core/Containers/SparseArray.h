#pragma once

#include <vector>
#include "Engine/Core/Types.h"
#include "Engine/Core/Platform.h"

template <typename T>
class TSparseArray
{
public:
    std::vector<T> SparseArray;

public:
    VL_FINLINE TSparseArray() :
        SparseArray()
    {}

    VL_FINLINE TSparseArray(VSizeType Size) :
        SparseArray(Size)
    {}

    VL_FINLINE TSparseArray(VSizeType Size, const T& Instance) :
        SparseArray(Size, Instance)
    {}

    VL_FINLINE void EmplaceBack(const T& Instance)
    {
        SparseArray.emplace_back(Instance);
    }

    VL_FINLINE T& GetBack(const T& Instance)
    {
        return SparseArray.back();
    }

    VL_FINLINE auto begin()
    {
        return SparseArray.begin();
    }

    VL_FINLINE auto end()
    {
        return SparseArray.end();
    }
};
