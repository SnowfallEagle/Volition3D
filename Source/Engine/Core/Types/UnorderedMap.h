#pragma once

#include <unordered_map>

namespace Volition
{

template<typename KeyT, typename ValueT>
class TUnorderedMap : public std::unordered_map<KeyT, ValueT>
{
public:
    using Super = std::unordered_map<KeyT, ValueT>;

public:
    TUnorderedMap() = default;

    TUnorderedMap(std::initializer_list<typename Super::value_type> InitList) : Super(InitList)
    {}
};

}