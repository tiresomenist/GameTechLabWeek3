#pragma once
#include "Core/Core.h"
#include "Core/Container/FString.h"

struct FName
{
public:
	FName();
	FName(const char* pStr);
	FName(FString Str);
	FName(const char* pStr, uint32 InInternalNumber);
	FName(FString Str, uint32 InInternalNumber);
	//이름 인덱스는 유지하고 내부 번호만 교체
	FName(const FName& BaseName, uint32 InInternalNumber);
	int32 Compare(const FName& Other) const;
	bool operator==(const FName& Other) const;
	bool operator!=(const FName& Other) const;

	FString ToString()const;

	// 동등성 비교에 사용하는 필드로 해시를 계산함
	std::size_t GetHash() const noexcept;

	// 접미사 없는 None 이름인지 확인함
	bool IsNone() const;

private:
	void RegisterBaseName(FString Str);

	int32 DisplayIndex = -1;
	int32 ComparisonIndex = -1;
	uint32 Number = 0;
};

namespace std
{
	template<>
	struct hash<FName>
	{
		std::size_t operator()(const FName& Value) const noexcept
		{
			return Value.GetHash();
		}
	};
}