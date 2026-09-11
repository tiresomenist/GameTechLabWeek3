#pragma	once
#include <deque>

#include "Core/Core.h"

template<typename T>
class TDeque
{
private:
	std::deque<T> Deque;
	
public:

	TDeque()
		: Deque{}
	{
	}

	TDeque(size_t Size)
		: Deque(Size)
	{
	}

	TDeque(const std::initializer_list<T>& List)
		: Deque(List)
	{
	}

	bool IsEmpty() const
	{
		return Deque.empty();
	}

	size_t Num() const
	{
		return Deque.size();
	}

	void PopFirst()
	{
		Deque.pop_front();
	}

	void PopLast()
	{
		Deque.pop_back();
	}

	void PushFirst(T& Item)
	{
		Deque.push_front(Item);
	}

	void PushLast(T& Item)
	{
		Deque.push_back(Item);
	}

	T& operator[](size_t Index)
	{
		return Deque[Index];
	}

	void Reset()
	{
		Deque.clear();
	}

	auto begin() { return Deque.begin(); }
	auto end() { return Deque.end(); }

	auto begin() const { return Deque.begin(); }
	auto end() const { return Deque.end(); }
};