#pragma once

#include <map>

namespace Volition
{

template<typename KeyT, typename ValueT>
class TMap : public std::map<KeyT, ValueT>
{
public:
    using Super = std::map<KeyT, ValueT>;

public:
    TMap() = default;

    TMap(std::initializer_list<typename Super::value_type> InitList) : Super(InitList)
    {}

};

}