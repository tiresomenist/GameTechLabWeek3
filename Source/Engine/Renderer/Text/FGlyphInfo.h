#pragma once

// 아틀라스에 구운 글자 하나에 대한 메타데이터
struct FGlyphInfo {
	float U0, V0, U1, V1;
	float XOffset, YOffset;
	float Width, Height;
	float XAdvance;
};