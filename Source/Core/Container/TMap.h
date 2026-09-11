#pragma once
#include <map>

template<typename KeyType, typename ValueType>
class TMap
{
public:
    ValueType& operator[](const KeyType& key)
    {
        return Map[key];
    }

    const ValueType* Find(const KeyType& key) const
    {
        auto it = Map.find(key);
        if (it != Map.end())
        {
            return &(it->second);
        }
        return nullptr;
    }

    void Clear()
    {
        Map.clear();
    }
private:
    std::map<KeyType, ValueType> Map;
};