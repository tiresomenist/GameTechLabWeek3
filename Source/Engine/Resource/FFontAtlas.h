#pragma once

#include "Engine/Resource/FTexture.h"
#include "stb/stb_truetype.h"
#include "Core/Container/TMap.h"
#include "Core/Container/TArray.h"

#include <cstdint>

// 한 글자의 아틀라스 내 위치와 배치 정보. 단위: 구울 때의 픽셀
struct FGlyph
{
	float U0, V0, U1, V1;       // 아틀라스 속 영역 (0~1)
	float XOff, YOff;           // 펜 위치 → 쿼드 좌상단
	float XOff2, YOff2;         // 펜 위치 → 쿼드 우하단
	float XAdvance;             // 다음 글자까지 펜 이동량
};

class FFontAtlas
{
public:
	bool Load(ID3D11Device* Device, const char* Path, float PixelHeight);

	const FGlyph* FindGlyph(uint32 Codepoint) const;
	const FTexture& GetTexture() const { return Texture; }
	float GetLineHeight() const { return Ascent - Descent + LineGap; }
	void Release();
	float GetDescent() const { return Descent; }                  // 기준선 아래 깊이 (음수)
	float GetAscent() const { return Ascent; }                    // 첫 줄 기준선 위치
	float GetBakedPixelHeight() const { return BakedPixelHeight; } // 픽셀 → 월드 스케일 계산

private:
	TArray<unsigned char> TtfBuffer;   // stbtt_fontinfo가 포인터로 참조 → 아틀라스와 수명 같이
	stbtt_fontinfo Info = {};

	TMap<uint32, FGlyph> Glyphs;
	FTexture Texture;

	float BakedPixelHeight = 0.f;
	float Ascent = 0.f, Descent = 0.f, LineGap = 0.f;   // 스케일 적용 후 픽셀
	// ASCII 95자 + KS X 1001 한글 2,350자 (32px + 패딩) → 약 2.7M px² 필요 → 2048² (R8, 4MB)
	static constexpr int AtlasSize = 2048;
};