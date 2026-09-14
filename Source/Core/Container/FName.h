#pragma once
#include "Core/Core.h"
#include "FString.h"

struct FName
{
	int32 DisplayIndex = -1;
	int32 ComparisonIndex = -1;

	FName() = default;
	FName(const char* pStr);
	FName(const FString& str);

	int32 GetComparisonIndex() const { return ComparisonIndex; }
	int32 GetDisplayIndex() const { return DisplayIndex; }

	int32 Compare(const FName& Other) const { return ComparisonIndex - Other.ComparisonIndex; }
	bool operator==(const FName& Other) const { return ComparisonIndex == Other.ComparisonIndex; }
	bool operator!=(const FName& Other) const { return ComparisonIndex != Other.ComparisonIndex; }
	bool operator<(const FName& Other) const { return ComparisonIndex < Other.ComparisonIndex; }
	bool operator>(const FName& Other) const { return ComparisonIndex > Other.ComparisonIndex; }

	FString ToString() const;
};

class FNamePool
{
public:
	static FNamePool* GetInstance();

	void FindOrAdd(const char* InStr, int32& OutDisplayIndex, int32& OutComparisonIndex);
	FString GetDisplayString(int32 Index) const;
	int32 GetNumNames() const;

private:
	FNamePool();
	~FNamePool();

	struct FNamePoolData* Data = nullptr;
};