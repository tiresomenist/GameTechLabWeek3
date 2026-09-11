#include "pch.h"
#include "FFontAtlas.h"
#include "Engine/Log.h"

#include <fstream>

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

	TArray<unsigned char> Bitmap(AtlasSize * AtlasSize);
	stbtt_packedchar Chars[95];

	stbtt_pack_context Ctx;
	stbtt_PackBegin(&Ctx, Bitmap.GetData(), AtlasSize, AtlasSize, 0, 1, nullptr);
	//                                                            │  └ 글자 사이 패딩 1px
	//                                                            └ stride 0 = 너비와 같게 촘촘히
	const int Ok = stbtt_PackFontRange(&Ctx, TtfBuffer.GetData(), 0, PixelHeight, 32, 95, Chars);
	stbtt_PackEnd(&Ctx);

	if (Ok == 0)
	{
		UE_LOG("[Font] Font Bake Failed");
		return false;
	}

	for (size_t i = 0; i < 95; i++)
	{
		const stbtt_packedchar& C = Chars[i];
		FGlyph Glyph;
		const float InvSize = 1.0f / AtlasSize;
		Glyph.U0 = C.x0 * InvSize;
		Glyph.U1 = C.x1 * InvSize;
		Glyph.V0 = C.y0 * InvSize;
		Glyph.V1 = C.y1 * InvSize;
		Glyph.XOff = C.xoff;
		Glyph.XOff2 = C.xoff2;
		Glyph.YOff = C.yoff;
		Glyph.YOff2 = C.yoff2;
		Glyph.XAdvance = C.xadvance;
		Glyphs[i + 32] = Glyph;
	}

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
