#pragma	once
#include <vector>
#include <stdexcept>
#include <functional>
#include <algorithm>

#include "Core/Core.h"

template<typename T>
class TArray
{
private:
	std::vector<T> Array;

public:

	TArray()
		: Array()
	{
	}

	TArray(size_t Size)
		: Array(Size)
	{
	}

	TArray(const std::initializer_list<T>& List)
		: Array(List)
	{
	}

	void Add(const T& Element)
	{
		Array.push_back(Element);
	}

	void Append(std::initializer_list<T> Elements)
	{
		Array.insert(Array.end(), Elements.begin(), Elements.end());
	}

	void Reserve(size_t Capacity)
	{
		Array.reserve(Capacity);
	}

	void Empty()
	{
		Array.clear();
	}

	bool IsEmpty() const
	{
		return Array.empty();
	}

	T Pop()
	{
		if (Array.empty()) throw std::runtime_error("Stack empty");
		T Top = Array.back();
		Array.pop_back();
		return Top;
	}

	int Num() const
	{
		return Array.size();
	}

	void SetNum(size_t Size)
	{
		Array.resize(Size);
	}

	T* GetData()
	{
		return Array.data();
	}

	const T* GetData() const
	{
		return Array.data();
	}

	void RemoveAt(size_t Index)
	{
		Array.erase(Array.begin() + Index);
	}

	T& operator[](size_t Index)
	{
		return Array[Index];
	}

	const T& operator[](size_t Index) const
	{
		return Array[Index];
	}

	void Sort(std::function<bool(const T&, const T&)> Compare = std::less<T>())
	{
		std::sort(Array.begin(), Array.end(), Compare);
	}

	void assign(T* first, T* last)
	{
		Array.assign(first, last);
	}

	void resize(size_t Index)
	{
		Array.resize(Index);
	}

	std::vector<T>& GetVector() { return Array; }

	auto begin() { return Array.begin(); }
	auto end() { return Array.end(); }

	auto begin() const { return Array.begin(); }
	auto end() const { return Array.end(); }
};
