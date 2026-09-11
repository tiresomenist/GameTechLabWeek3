#include "GAllocator.h"
#include "Engine/Core.h"
#include "Engine/Log.h"

#include <format>
#include <memory>
#include <stdexcept>
#include <cstdlib>

void* GAllocator::Allocate(size_t Size, size_t Alignment)
{
	//Size 유효성 체크
	if (Size == 0) return nullptr;

	// Align이 0이거나 2의 거듭제곱이어야만 함
	if (Alignment == 0) { return nullptr; }
	if ((Alignment & (Alignment - 1)) != 0) { return nullptr; }

	if (Size > SIZE_MAX - sizeof(FAllocationHeader) - (Alignment - 1)){ return nullptr;}
	std::size_t TotalRawSize = sizeof(FAllocationHeader) + (Alignment - 1) + Size;

	if (TotalRawSize < 0) { return nullptr; }

	void* RawPtr = std::malloc(TotalRawSize);
	if (RawPtr == nullptr)
	{
		throw std::bad_alloc();
	}

	// FAllocationHeader 이후의 첫 위치를 찾기
	std::uintptr_t BeginPosition = reinterpret_cast<std::uintptr_t>(RawPtr)
		+ sizeof(FAllocationHeader);

	// 요청한 메모리가 시작될 위치를 찾기
	std::uintptr_t MemoryPosition = (BeginPosition + Alignment - 1) & ~(Alignment - 1);

	// 헤더가 시작될 위치를 찾기
	std::uintptr_t HeaderPosition = MemoryPosition - sizeof(FAllocationHeader);

	// 메모리 주소 -> 포인터 변환
	void* MemoryPtr = reinterpret_cast<void*>(MemoryPosition);
	FAllocationHeader* HeaderPtr = reinterpret_cast<FAllocationHeader*>(HeaderPosition);

	// FAllocationHeader 생성자 호출 후 초기화
	std::construct_at(
		HeaderPtr,
		FAllocationHeader
		{
				.RawPtr = RawPtr,
				.AllocationSize = TotalRawSize,
				.Alignment = Alignment,
		}
	);

	// 통계 정보에 등록
	TotalAllocationBytes += TotalRawSize;
	++TotalAllocationCount;

	// 최종 메모리 포인터 반환
	return MemoryPtr;
}

void GAllocator::Free(void* Ptr)
{
	if (Ptr == nullptr) { return; }

	// 메모리에서 헤더 포인터 추출
	std::uintptr_t PtrPosition = reinterpret_cast<std::uintptr_t>(Ptr);
	std::uintptr_t HeaderPosition = PtrPosition - sizeof(FAllocationHeader);

	FAllocationHeader* Header = reinterpret_cast<FAllocationHeader*>(HeaderPosition);

	size_t Size = Header->AllocationSize;
	void* RawPtr = Header->RawPtr;

	// FAllocationHeader 소멸자 호출
	std::destroy_at(Header);

	// 통계 정보에 등록
	TotalAllocationBytes -= Size;
	--TotalAllocationCount;

	// 최종 메모리 반환
	std::free(RawPtr);
}