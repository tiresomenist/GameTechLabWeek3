#include "pch.h"
#include "FName.h"

#include <charconv>
#include <mutex>

namespace
{
	class FNameTable
	{
	public:
		FNameTable()
		{
			Names.emplace_back("None");
			NameToIndex.emplace("None", 0);
		}

		uint32 FindOrAdd(FStringView Name)
		{
			if (Name.empty()) return 0;

			const FString Key(Name);
			std::lock_guard Lock(Mutex);
			if (const auto Found = NameToIndex.find(Key); Found != NameToIndex.end())
			{
				return Found->second;
			}

			const uint32 NewIndex = static_cast<uint32>(Names.size());
			Names.push_back(Key);
			NameToIndex.emplace(Names.back(), NewIndex);
			return NewIndex;
		}

		FString GetName(uint32 Index) const
		{
			std::lock_guard Lock(Mutex);
			return Index < Names.size() ? Names[Index] : Names[0];
		}

	private:
		mutable std::mutex Mutex;
		std::unordered_map<FString, uint32> NameToIndex;
		std::vector<FString> Names;
	};

	FNameTable& GetNameTable()
	{
		static FNameTable Table;
		return Table;
	}

	void ParseNameNumber(FStringView Name, FStringView& OutBaseName, uint32& OutNumber)
	{
		OutBaseName = Name;
		OutNumber = 0;

		const size_t Separator = Name.rfind('_');
		if (Separator == FStringView::npos || Separator == 0 || Separator + 1 == Name.size()) return;

		uint32 DisplayNumber = 0;
		const char* NumberStart = Name.data() + Separator + 1;
		const char* NumberEnd = Name.data() + Name.size();
		const auto [End, Error] = std::from_chars(NumberStart, NumberEnd, DisplayNumber);
		if (Error != std::errc{} || End != NumberEnd || DisplayNumber == (std::numeric_limits<uint32>::max)()) return;

		OutBaseName = Name.substr(0, Separator);
		OutNumber = DisplayNumber + 1;
	}
}

FName::FName(FStringView Name)
{
	FStringView BaseName;
	ParseNameNumber(Name, BaseName, Number);
	Index = GetNameTable().FindOrAdd(BaseName);
}

FString FName::ToString() const
{
	FString Name = GetNameTable().GetName(Index);
	if (Number == 0) return Name;

	return Name + '_' + std::to_string(Number - 1);
}
