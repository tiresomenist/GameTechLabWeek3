#include "pch.h"
#include "Core/Name/FName.h"
#include "Core/Container/TMap.h"
#include "Core/Container/TArray.h"

#include <charconv>
#include <format>
#include <limits>
#include <stdexcept>
#include <system_error>

namespace
{
    struct FNamePool
    {
        TMap<FString, uint32> NameToIndex;
        TArray<FString> IndexToName;
        int32 FindOrAdd(const FString& Text)
        {
            // 이미 등록된 문자열은 기존 인덱스를 반환함
            if (const uint32* FoundIndex = NameToIndex.Find(Text))
            {
                return static_cast<int32>(*FoundIndex);
            }

            const int32 NewIndex = IndexToName.Num();

            if (NewIndex >= (std::numeric_limits<int32>::max)())
            {
                throw std::length_error("FName pool 용량 초과");
            }
            // 새 문자열을 배열의 마지막에 추가함
            IndexToName.Add(Text);

            try
            {
                // 문자열에서 인덱스를 찾는 검색 정보를 추가함
                const bool bAdded = NameToIndex.Add(Text,static_cast<uint32>(NewIndex));
                if (!bAdded)
                {
                    throw std::logic_error("FName pool 중복 입력");
                }
            }
            catch (...)
            {
                // 맵 추가 실패 시 배열 추가도 되돌림
                IndexToName.RemoveAt(NewIndex);
                throw;
            }
            return NewIndex;
        }
        FString GetString(int32 Index) const
        {
            if (Index < 0 || Index >= IndexToName.Num())
            {
                throw std::out_of_range("잘못된 FName 인덱스 접근");
            }
            return IndexToName[Index];
        }
    };

    FNamePool& GetNamePool()
    {
        static FNamePool Pool;
        return Pool;
    }

    bool TrySplitNameNumber(FString& InOutName, uint32& OutInternalNumber)
    {
        const auto Separator = InOutName.rfind('_');

        // 구분자가 없거나 기본 이름이 비어 있으면 분리하지 않음
        if (Separator == FString::npos || Separator == 0)
        {
            return false;
        }

        const FStringView NumberText = FStringView(InOutName).substr(Separator + 1);

        if (NumberText.empty())
        {
            return false;
        }

        // 선행 0이 있는 접미사는 원래 표기 보존을 위해 분리하지 않음
        if (NumberText.size() > 1 && NumberText.front() == '0')
        {
            return false;
        }

        // 접미사가 십진수 숫자로만 구성됐는지 검사함
        for (char Character : NumberText)
        {
            if (Character < '0' || Character > '9')
            {
                return false;
            }
        }

        uint32 ExternalNumber = 0;

        const char* Begin = NumberText.data();
        const char* End = Begin + NumberText.size();

        //숫자로 변환
        const auto Result = std::from_chars(Begin,End,ExternalNumber,10);

        if (Result.ec != std::errc{} || Result.ptr != End)
        {
            return false;
        }

        // 내부 Number로 변환할 때의 오버플로를 방지함
        if (ExternalNumber == (std::numeric_limits<uint32>::max)())
        {
            return false;
        }

        OutInternalNumber = ExternalNumber + 1;

        // 숫자 해석 성공 후 기본 이름만 남김
        InOutName.resize(Separator);

        return true;
    }
}
FName::FName()
    : FName("None")
{
}
FName::FName(const char* pStr)
    : FName(FString(pStr ? pStr : ""))
{
}

FName::FName(FString Str)
{
    // Number는 멤버 기본값인 0에서 시작함
    TrySplitNameNumber(Str, Number);

    RegisterBaseName(Str);
}

FName::FName(const char* pStr, uint32 InInternalNumber)
    : FName(FString(pStr ? pStr : ""),InInternalNumber)
{
}

// 만약 숫자 접미사를 명시적으로 선언하면, 해당 값이 우선 적용됨.
FName::FName(FString Str, uint32 InInternalNumber)
    : Number(InInternalNumber)
{
    // 명시적으로 숫자를 지정한 경우 기본 이름을 그대로 등록함
    uint32 temp;
    
    if (TrySplitNameNumber(Str, temp)) {
        //만약 명시적으로 숫자를 지정했는데도 문자열에 숫자 접미사가 있었다면 어떻게 처리를 해야하나..
        // 지금 코드 기준으론 교체된다. 명시적 입력을 더 중시한다는 정책으로 감.
    }
    RegisterBaseName(Str);
}

int32 FName::Compare(const FName& Other) const
{
    //서로 다른 String일때
    if (ComparisonIndex != Other.ComparisonIndex)
    {
        const FNamePool& Pool = GetNamePool();

        const FString Left = Pool.GetString(ComparisonIndex);
        const FString Right = Pool.GetString(Other.ComparisonIndex);

        const int Result = Left.compare(Right);

        if (Result < 0)
        {
            return -1;
        }

        if (Result > 0)
        {
            return 1;
        }
    }

    //서로 같은 String일때
    if (Number == Other.Number)
    {
        return 0;
    }

    return Number < Other.Number ? -1 : 1;
}

bool FName::operator==(const FName& Other) const
{
    return ComparisonIndex == Other.ComparisonIndex && Number == Other.Number;
}

bool FName::operator!=(const FName& Other) const
{
    return !(*this == Other);
}

FString FName::ToString() const
{
    FString Result = GetNamePool().GetString(DisplayIndex);

    if (Number != 0)
    {
        // 내부 Number를 화면에 표시할 숫자로 변환함
        Result += std::format("_{}", Number - 1);
    }

    return Result;
}

void FName::RegisterBaseName(FString Str)
{
    if (Str.empty())
    {
        Str = "None";
    }

    const FString DisplayText = Str;

    // 비교용 기본 이름의 영문 대소문자를 통일함
    for (char& Character : Str)
    {
        if (Character >= 'A' && Character <= 'Z')
        {
            Character = static_cast<char>(Character + 32);
        }
    }

    FNamePool& Pool = GetNamePool();

    ComparisonIndex = Pool.FindOrAdd(Str);
    DisplayIndex = Pool.FindOrAdd(DisplayText);
}
