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

	int32 Compare(const FName& Other) const;
	bool operator==(const FName& Other) const;
	bool operator!=(const FName& Other) const;

	FString ToString()const;

private:
	void RegisterBaseName(FString Str);

	int32 DisplayIndex = -1;
	int32 ComparisonIndex = -1;
	uint32 Number = 0;
};