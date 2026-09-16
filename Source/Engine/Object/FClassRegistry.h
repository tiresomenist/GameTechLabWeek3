#pragma once

#include "Core/Container/TArray.h"
#include "Core/Container/FString.h"
#include "Core/Name/FName.h"

struct FClassType;

class FClassRegistry
{
public:
	/// <summary>
	/// 클래스 정의를 추가합니다.
	/// 주의: 이 함수는 UCLASS를 통해 호출되므로 직접 이 함수를 호출하지 말것
	/// </summary>
	/// <param name="Type">등록하려는 FClassType</param>
	static void* __INTERNAL__Add(FClassType* Type);

	/// <summary>
	/// 주어진 타입 FName으로 FClassType을 검색합니다.
	/// </summary>
	/// <param name="TypeName">찾고자 하는 FClassType의 FName</param>
	/// <returns></returns>
	static FClassType* FindClassType(const FName& TypeName);
};