#pragma once

#include <map>

template<typename KeyType, typename ValueType>
class TMap
{
private:
    std::map<KeyType, ValueType> Map;

public:
    bool Add(const KeyType& Key, const ValueType& Value)
    {
        const auto Result = Map.emplace(Key, Value);
        return Result.second;
    }

    ValueType* Find(const KeyType& Key)
    {
        const auto Iterator = Map.find(Key);

        if (Iterator == Map.end())
        {
            return nullptr;
        }

        return &Iterator->second;
    }

    const ValueType* Find(const KeyType& Key) const
    {
        const auto Iterator = Map.find(Key);

        if (Iterator == Map.end())
        {
            return nullptr;
        }

        return &Iterator->second;
    }
    auto begin() { return Map.begin(); }
    auto end() { return Map.end(); }

    auto begin() const { return Map.begin(); }
    auto end() const { return Map.end(); }

    void Empty()
    {
        Map.clear();
    }
};