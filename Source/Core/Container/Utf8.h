#pragma once

#include "Core/Core.h"
#include "Core/Container/FString.h"
#include "Core/Container/TArray.h"

namespace Utf8
{
	// U+FFFD: 유니코드가 정한 "잘못된 문자" 표시용 코드포인트
	constexpr uint32 Replacement = 0xFFFD;

	// Text[Index]에서 한 글자를 읽어 OutCodepoint에 넣고 Index를 다음 글자로 옮긴다.
	// 잘못된 바이트면 false를 반환하고 Index는 1바이트만 전진한다.
	inline bool DecodeNext(FStringView Text, size_t& Index, uint32& OutCodepoint)
	{
		unsigned char Ch = Text[Index];
		uint32 Bytes = 0;
		uint32 Cp = 0;
		uint32 Min = 0;
		bool valid = true;

		if (Ch < 0x80)
		{
			Bytes = 1;
			Cp = Ch;
			Min = 0;
		}
		else if ((Ch & 0xE0) == 0xC0)
		{
			Bytes = 2;
			Cp = Ch & 0x1F;
			Min = 0x80;
		}
		else if ((Ch & 0xF0) == 0xE0)
		{
			Bytes = 3;
			Cp = Ch & 0x0F;
			Min = 0x800;
		}
		else if ((Ch & 0xF8) == 0xF0)
		{
			Bytes = 4;
			Cp = Ch & 0x07;
			Min = 0x10000;
		}
		else
		{
			valid = false;
		}

		for (uint32 i = 1; i < Bytes; i++)
		{
			if (Index + i >= Text.length())
			{
				valid = false;
				break;
			}

			Ch = Text[Index + i];
			if ((Ch & 0xC0) != 0x80) { valid = false; break; }
			Cp = (Cp << 6) | (Ch & 0x3F);
		}

		if (Cp < Min || 0xD800 <= Cp && Cp <= 0xDFFF || Cp > 0x10FFFF)
		{
			valid = false;
		}

		if (valid)
		{
			OutCodepoint = Cp;
			Index += Bytes;
			return true;
		}

		Index += 1;
		return false;
	}

	// 문자열 전체 → 코드포인트 배열. 잘못된 바이트는 Replacement로.
	inline TArray<uint32> Decode(FStringView Text)
	{
		TArray<uint32> Cps;
		size_t Index = 0;

		while (Index < Text.length())
		{
			uint32 Codepoint = Replacement;
			if (DecodeNext(Text, Index, Codepoint))
			{
				Cps.Add(Codepoint);
			}
			else
			{
				Cps.Add(Codepoint);
			}
		}

		return Cps;
	}
}