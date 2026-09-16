#pragma once

#include <functional>
#include "Core/Core.h"
#include "Core/Container/FString.h"
#include "Core/Name/FName.h"

// 전방 선언
class UObject;
struct FClassType;
struct FObjectCreateInfo;

// UObject 생성자 타입
using Constructor = std::function<UObject* (const FObjectCreateInfo&)>;

struct FClassType
{
    // 객체를 직렬화/역직렬화 할 때 사용되는 이름입니다.
    const FName Name;

	// UI에서 재사용하는 클래스 표시 문자열임
	const FString DisplayName = Name.ToString();
	
	// TypeInfo로 객체를 생성할 때 사용되는 객체 생성 함수입니다.
    const Constructor ClassConstructor;

    // 객체의 상속 구조를 파악할 때 사용되는 부모 포인터입니다.
    const FClassType* ParentClassType = nullptr;

    bool IsA(FClassType* ClassType) const
    {
		const FClassType* CurrentType = this;

		// 포인터 노드를 순회하며 타입을 검색합니다.
		while (CurrentType != nullptr)
		{
			if (CurrentType == ClassType)
			{
				return true;
			}

			CurrentType = CurrentType->ParentClassType;
		}

		return false;
    }
};
