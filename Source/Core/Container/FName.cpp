#include "pch.h"
#include "FName.h"
#include "TArray.h"
#include "TMap.h"

struct FNamePoolData
{
    TArray<FString> DisplayTable;
    TArray<FString> ComparisonTable;
    TMap<FString, int32> ComparisonMap;
};


FNamePool::FNamePool()
{
    Data = new FNamePoolData();
}

FNamePool::~FNamePool()
{
    delete Data;
    Data = nullptr;
}

FNamePool* FNamePool::GetInstance()
{
	static FNamePool Instance;
	return &Instance;
}

void FNamePool::FindOrAdd(const char* InStr, int32& OutDisplayIndex, int32& OutComparisonIndex)
{
    FString Raw = InStr ? InStr : "";
    FString Lower = Raw;

    for (char& C : Lower)
    {
        unsigned char UC = static_cast<unsigned char>(C);
        if (UC >= 'A' && UC <= 'Z')
        {
            C = static_cast<char>(std::tolower(UC));
        }
    }

    int32* FoundIndex = Data->ComparisonMap.Find(Lower);
    if (FoundIndex != nullptr)
    {
        OutComparisonIndex = *FoundIndex;
    }
    else
    {
        OutComparisonIndex = static_cast<int32>(Data->ComparisonTable.Num());
        Data->ComparisonTable.Add(Lower);
        Data->ComparisonMap.Add(Lower, OutComparisonIndex);
    }

    OutDisplayIndex = static_cast<int32>(Data->DisplayTable.Num());
    Data->DisplayTable.Add(Raw);
}

FString FNamePool::GetDisplayString(int32 Index) const
{
    if (Index >= 0 && Index < static_cast<int32>(Data->DisplayTable.Num()))
    {
        return Data->DisplayTable[Index];
    }
    return FString("");
}

int32 FNamePool::GetNumNames() const
{
    return static_cast<int32>(Data->ComparisonTable.Num());
}

FName::FName(const char* InStr)
{
    FNamePool::GetInstance()->FindOrAdd(InStr, DisplayIndex, ComparisonIndex);
}

FName::FName(const FString& InStr)
    : FName(InStr.c_str())
{
}

FString FName::ToString() const
{
    return FNamePool::GetInstance()->GetDisplayString(DisplayIndex);
}