#pragma once

#include "Core/Core.h"
#include <cstddef>

// 아주 간단한 Allocator
// 
// TODO: Linear Allocator, Pool Allocator, Stack Allocator 등
// 상황 및 객체 생명주기에 맞는 다양한 Allocator 구현해보기.

struct FAllocationHeader
{
	void* RawPtr;
	size_t AllocationSize;
	size_t Alignment;
};


class GAllocator
{
private:
	inline static size_t TotalAllocationBytes;
	inline static size_t TotalAllocationCount;

public:
	/// <summary>
	/// 새로운 메모리 공간을 할당
	/// </summary>
	/// <param name="Size">할당 받을 메모리의 크기 (바이트)</param>
	/// <param name="Alignment">할당 받을 메모리의 정렬 (바이트)</param>
	/// <returns>할당된 포인터 (초기화 필요)</returns>
	static void* Allocate(size_t Size, size_t Alignment = alignof(std::max_align_t));
	
	/// <summary>
	/// 할당된 메모리 공간을 반환
	/// </summary>
	/// <param name="Ptr">반환할 메모리</param>
	static void Free(void* Ptr);

	static size_t GetTotalAllocationBytes() { return TotalAllocationBytes; };
	static size_t GetTotalAllocationCount() { return TotalAllocationCount; };
};