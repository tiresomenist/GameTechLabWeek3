#pragma once

#include <unordered_map>

template<typename KeyType, typename ValueType>
class TMap
{
private:
    std::unordered_map<KeyType, ValueType> Map;

public:
    bool Add(const KeyType& Key, const ValueType& Value)
    {
        const auto Result = Map.emplace(Key, Value);
        return Result.second;
    }

    bool Contains(const KeyType& Key) const
    {
        return Map.find(Key) != Map.end();
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

    ValueType& operator[](const KeyType& Key)
    {
        return Map[Key];
    }

    bool Remove(const KeyType& Key)
    {
        return Map.erase(Key) > 0;
    }

    int Num() const
    {
        return Map.size();
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