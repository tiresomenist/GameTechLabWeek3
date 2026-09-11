#include "pch.h"
#include "FFontAtlas.h"
#include "Engine/Log.h"

#include <fstream>
#include <chrono>

namespace
{
	// KS X 1001(완성형) 한글 2,350자의 유니코드 코드포인트.
	// CP949에서 한글 음절은 선행 바이트 0xB0~0xC8, 후행 바이트 0xA1~0xFE 조합(25 × 94 = 2,350)에 있다.
	// 일상 문장의 대부분을 덮지만 "똠", "쌰" 같은 글자는 빠져 있다 → FindGlyph 실패 → '?'로 대체
	TArray<int> MakeKsx1001HangulCodepoints()
	{
		TArray<int> Codepoints;
		for (int Lead = 0xB0; Lead <= 0xC8; ++Lead)
		{
			for (int Trail = 0xA1; Trail <= 0xFE; ++Trail)
			{
				const char Bytes[2] = { static_cast<char>(Lead), static_cast<char>(Trail) };
				wchar_t Wide = 0;
				if (MultiByteToWideChar(949, MB_ERR_INVALID_CHARS, Bytes, 2, &Wide, 1) == 1
					&& Wide >= 0xAC00 && Wide <= 0xD7A3)   // 한글 음절 블록만
				{
					Codepoints.Add(static_cast<int>(Wide));
				}
			}
		}
		return Codepoints;
	}
}

bool FFontAtlas::Load(ID3D11Device* Device, const char* Path, float PixelHeight)
{
	// ate: 파일 끝에서 열어서 tellg()로 크기를 바로 얻는다
	std::ifstream File(Path, std::ios::binary | std::ios::ate);
	if (!File)
	{
		UE_LOG("[Font] 파일을 열 수 없습니다: {}", Path);
		return false;
	}

	const std::streamsize Size = File.tellg();
	File.seekg(0);
	TtfBuffer.resize(static_cast<size_t>(Size));
	File.read(reinterpret_cast<char*>(TtfBuffer.GetData()), Size);

	const int NumFonts = stbtt_GetNumberOfFonts(TtfBuffer.GetData());
	const int Offset = stbtt_GetFontOffsetForIndex(TtfBuffer.GetData(), 0);

	if (!stbtt_InitFont(&Info, TtfBuffer.GetData(), Offset))
	{
		UE_LOG("[Font] InitFont 실패: {}", Path);
		return false;
	}

	// 덤: 코드포인트 → 글리프 인덱스 매핑(cmap) 확인. 0이면 폰트에 없는 글자
	const int GlyphA = stbtt_FindGlyphIndex(&Info, 'A');
	const int GlyphGa = stbtt_FindGlyphIndex(&Info, 0xAC00); // '가'

	UE_LOG("[Font] InitFont OK: {} ({} bytes, 폰트 {}개, 'A'={}, '가'={})",
		Path, Size, NumFonts, GlyphA, GlyphGa);

	const float Scale = stbtt_ScaleForPixelHeight(&Info, PixelHeight);
	int A, D, G;
	stbtt_GetFontVMetrics(&Info, &A, &D, &G);
	Ascent = A * Scale;  Descent = D * Scale;  LineGap = G * Scale;

	UE_LOG("[Scale] A: {}, D: {}, G: {}", Ascent, Descent, LineGap);

	// ── 구울 글자 목록: ASCII(연속 범위) + 한글(코드포인트 배열) ──
	TArray<int> Hangul = MakeKsx1001HangulCodepoints();
	if (Hangul.Num() != 2350)
	{
		UE_LOG("[Font] 경고: KS X 1001 한글 {}자 (기대 2350자). CP949 변환을 확인하세요", Hangul.Num());
	}

	TArray<stbtt_packedchar> AsciiChars(95);
	TArray<stbtt_packedchar> HangulChars(Hangul.Num());

	stbtt_pack_range Ranges[2] = {};
	Ranges[0].font_size = PixelHeight;
	Ranges[0].first_unicode_codepoint_in_range = 32;           // 32 ~ 126, 연속
	Ranges[0].num_chars = 95;
	Ranges[0].chardata_for_range = AsciiChars.GetData();

	Ranges[1].font_size = PixelHeight;
	Ranges[1].array_of_unicode_codepoints = Hangul.GetData();   // 연속이 아닌 목록
	Ranges[1].num_chars = Hangul.Num();
	Ranges[1].chardata_for_range = HangulChars.GetData();

	const int NumRanges = Hangul.Num() > 0 ? 2 : 1;

	// ── 굽기 ──
	TArray<unsigned char> Bitmap(AtlasSize * AtlasSize);
	stbtt_pack_context Ctx;
	if (!stbtt_PackBegin(&Ctx, Bitmap.GetData(), AtlasSize, AtlasSize, 0, 1, nullptr))
	//                                                                │  └ 글자 사이 패딩 1px
	//                                                                └ stride 0 = 너비와 같게 촘촘히
	{
		UE_LOG("[Font] PackBegin 실패");
		return false;
	}

	const auto BakeStart = std::chrono::steady_clock::now();
	const int Ok = stbtt_PackFontRanges(&Ctx, TtfBuffer.GetData(), 0, Ranges, NumRanges);
	stbtt_PackEnd(&Ctx);
	const auto BakeMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - BakeStart).count();

	if (Ok == 0)
	{
		UE_LOG("[Font] Font Bake Failed: {}x{} 아틀라스에 글자가 다 들어가지 않습니다", AtlasSize, AtlasSize);
		return false;
	}

	// ── stbtt_packedchar → FGlyph. 범위마다 i번째 결과가 어떤 코드포인트인지 되찾는다 ──
	const float InvSize = 1.0f / AtlasSize;
	for (int R = 0; R < NumRanges; ++R)
	{
		const stbtt_pack_range& Range = Ranges[R];
		for (int i = 0; i < Range.num_chars; ++i)
		{
			const uint32 Codepoint = Range.array_of_unicode_codepoints
				? static_cast<uint32>(Range.array_of_unicode_codepoints[i])
				: static_cast<uint32>(Range.first_unicode_codepoint_in_range + i);

			const stbtt_packedchar& C = Range.chardata_for_range[i];
			FGlyph Glyph;
			Glyph.U0 = C.x0 * InvSize;
			Glyph.U1 = C.x1 * InvSize;
			Glyph.V0 = C.y0 * InvSize;
			Glyph.V1 = C.y1 * InvSize;
			Glyph.XOff = C.xoff;
			Glyph.XOff2 = C.xoff2;
			Glyph.YOff = C.yoff;
			Glyph.YOff2 = C.yoff2;
			Glyph.XAdvance = C.xadvance;
			Glyphs[Codepoint] = Glyph;
		}
	}

	UE_LOG("[Font] Baked {} glyphs into {}x{} in {} ms", Glyphs.size(), AtlasSize, AtlasSize, BakeMs);

	BakedPixelHeight = PixelHeight;
	return Texture.Create(Device, AtlasSize, AtlasSize, DXGI_FORMAT_R8_UNORM, 1, Bitmap.GetData());
}

const FGlyph* FFontAtlas::FindGlyph(uint32 Codepoint) const
{
	auto it = Glyphs.find(Codepoint);
	return (it == Glyphs.end() ? nullptr : &it->second);
}

void FFontAtlas::Release()
{
	Texture.Release();
	Glyphs.clear();
}
