#pragma once

#include <vector>

template <typename T>
class VSparseArray
{
public:
    std::vector<T> SparseArray;

public:
    VSparseArray() :
        SparseArray()
    {}

    VSparseArray(VSizeType Size) :
        SparseArray(Size)
    {}

    VSparseArray(VSizeType Size, const T& Instance) :
        SparseArray(Size, Instance)
    {}
};
