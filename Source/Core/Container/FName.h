#pragma once

#include "Core/Container/FString.h"
#include "Core/Core.h"

// 실행 중 전역 이름 테이블을 공유하는 경량 이름 식별자입니다.
struct FName
{
	// 0은 접미사 번호가 없음을 뜻한다. 0보다 큰 값 N은 표시 문자열의 _{N - 1} 접미사다.
	uint32 Index = 0;
	uint32 Number = 0;

	FName() = default;
	explicit FName(FStringView Name);

	bool operator==(const FName& Other) const
	{
		return Index == Other.Index && Number == Other.Number;
	}

	bool operator!=(const FName& Other) const { return !(*this == Other); }
	bool IsNone() const { return Index == 0 && Number == 0; }

	FString ToString() const;
};
