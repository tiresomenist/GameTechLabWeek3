#pragma once
#include <map>

template<typename KeyType, typename ValueType>
class TMap
{
private:
	std::map<KeyType, ValueType> Map;

public:
	// Add a new key or replace the value associated with an existing key.
	ValueType& Add(const KeyType& Key, const ValueType& Value)
	{
		return Map.insert_or_assign(Key, Value).first->second;
	}

	// A missing key returns nullptr without inserting a default value.
	ValueType* Find(const KeyType& Key)
	{
		auto It = Map.find(Key);
		return It != Map.end() ? &It->second : nullptr;
	}

	const ValueType* Find(const KeyType& Key) const
	{
		auto It = Map.find(Key);
		return It != Map.end() ? &It->second : nullptr;
	}

	void Empty() { Map.clear(); }
	bool IsEmpty() const { return Map.empty(); }
	int Num() const { return static_cast<int>(Map.size()); }
};
