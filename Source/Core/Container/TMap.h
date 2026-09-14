#pragma once
#include <map>

template<typename KeyType, typename ValueType>
class TMap
{
private:
	std::map<KeyType, ValueType> Elements;
	
public:
	TMap() = default;

    void Add(const KeyType& Key, const ValueType& Value)
    {
        Elements[Key] = Value;
    }

    // 키 존재 여부 확인
    bool Contains(const KeyType& Key) const
    {
        return Elements.find(Key) != Elements.end();
    }

    // 값 찾기
    ValueType* Find(const KeyType& Key)
    {
        auto It = Elements.find(Key);
        if (It != Elements.end())
        {
            return &(It->second);
        }
        return nullptr;
    }

    const ValueType* Find(const KeyType& Key) const
    {
        auto It = Elements.find(Key);
        if (It != Elements.end())
        {
            return &(It->second);
        }
        return nullptr;
    }

    // 키를 통한 직접 접근
    ValueType& operator[](const KeyType& Key)
    {
        return Elements[Key];
    }

    // 원소 삭제
    bool Remove(const KeyType& Key)
    {
        return Elements.erase(Key) > 0;
    }

    // 요소 전체 비우기
    void Empty()
    {
        Elements.clear();
    }

    // 원소 개수 반환
    int32 Num() const
    {
        return static_cast<int32>(Elements.size());
    }

    // 범위 기반 for문
    auto begin() { return Elements.begin(); }
    auto end() { return Elements.end(); }
    auto begin() const { return Elements.begin(); }
    auto end() const { return Elements.end(); }
};