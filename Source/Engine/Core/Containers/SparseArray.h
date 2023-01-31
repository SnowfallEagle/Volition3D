#pragma once

#include <vector>

template <typename T>
class TSparseArray
{
public:
    std::vector<T> SparseArray;

public:
    TSparseArray() :
        SparseArray()
    {}

    TSparseArray(VSizeType Size) :
        SparseArray(Size)
    {}

    TSparseArray(VSizeType Size, const T& Instance) :
        SparseArray(Size, Instance)
    {}
};
