#include "pch.h"
#include "Engine/Renderer/Text/FTextMeshBuilder.h"

namespace
{
	// UTF-8 바이트 시퀀스를 유니코드 코드포인트(char32_t) 배열로 디코딩.
	// 한글처럼 3바이트짜리 문자도 여기서 하나의 코드포인트로 합쳐진다.
	TArray<char32_t> DecodeUTF8(const FString& Text)
	{
		TArray<char32_t> Result;
		size_t i = 0;
		while (i < Text.size())
		{
			const unsigned char c = static_cast<unsigned char>(Text[i]);
			char32_t Codepoint = 0;
			int ExtraBytes = 0;

			if ((c & 0x80) == 0x00) { Codepoint = c; ExtraBytes = 0; }
			else if ((c & 0xE0) == 0xC0) { Codepoint = c & 0x1F; ExtraBytes = 1; }
			else if ((c & 0xF0) == 0xE0) { Codepoint = c & 0x0F; ExtraBytes = 2; }
			else if ((c & 0xF8) == 0xF0) { Codepoint = c & 0x07; ExtraBytes = 3; }
			else { ++i; continue; } // 잘못된 시작 바이트는 건너뜀

			if (i + ExtraBytes >= Text.size()) break;

			bool bValid = true;
			for (int j = 1; j <= ExtraBytes; ++j)
			{
				const unsigned char cc = static_cast<unsigned char>(Text[i + j]);
				if ((cc & 0xC0) != 0x80) { bValid = false; break; }
				Codepoint = (Codepoint << 6) | (cc & 0x3F);
			}

			if (bValid)
				Result.Add(Codepoint);

			i += ExtraBytes + 1;
		}
		return Result;
	}
}

void FTextMeshBuilder::AppendString(
	TArray<FVertexText>& OutVertices,
	const FString& Text,
	const FVector& WorldPosition,
	const FVector& Right,
	const FVector& Up,
	const FFontAtlas& Atlas,
	float WorldUnitsPerPixel)
{
	TArray<char32_t> Codepoints = DecodeUTF8(Text);

	// 가로 중앙 정렬을 위해 전체 폭을 먼저 구한다.
	float TotalWidth = 0.0f;
	for (int i = 0; i < Codepoints.Num(); ++i)
	{
		if (const FGlyphInfo* Glyph = Atlas.FindGlyph(Codepoints[i]))
			TotalWidth += Glyph->XAdvance;
	}

	float PenX = -TotalWidth * 0.5f;
	const float PenY = 0.0f;

	for (int i = 0; i < Codepoints.Num(); ++i)
	{
		const FGlyphInfo* Glyph = Atlas.FindGlyph(Codepoints[i]);
		if (!Glyph)
			continue; // 아틀라스에 없는 글자는 스킵

		// 로컬(픽셀 단위) quad 좌표. stb는 Y가 아래로 증가하므로 부호를 반전해서 위로 향하게 한다.
		const float x0 = PenX + Glyph->XOffset;
		const float x1 = x0 + Glyph->Width;
		const float y0 = PenY - Glyph->YOffset;
		const float y1 = y0 - Glyph->Height;

		// 로컬 2D 좌표를 카메라 Right/Up 축으로 확장 → 월드 좌표. 이게 빌보드의 핵심.
		auto ToWorld = [&](float lx, float ly)
		{
			return WorldPosition + Right * (lx * WorldUnitsPerPixel) + Up * (ly * WorldUnitsPerPixel);
		};

		const FVector P0 = ToWorld(x0, y0); // 좌상
		const FVector P1 = ToWorld(x1, y0); // 우상
		const FVector P2 = ToWorld(x1, y1); // 우하
		const FVector P3 = ToWorld(x0, y1); // 좌하

		auto PushVertex = [&](const FVector& P, float U, float V)
		{
			FVertexText Vert;
			Vert.X = P.X; Vert.Y = P.Y; Vert.Z = P.Z;
			Vert.U = U; Vert.V = V;
			Vert.R = 1.0f; Vert.G = 1.0f; Vert.B = 1.0f; Vert.A = 1.0f;
			OutVertices.Add(Vert);
		};

		PushVertex(P0, Glyph->U0, Glyph->V0);
		PushVertex(P1, Glyph->U1, Glyph->V0);
		PushVertex(P2, Glyph->U1, Glyph->V1);
		PushVertex(P3, Glyph->U0, Glyph->V1);

		PenX += Glyph->XAdvance;
	}
}

TArray<FVertexText> FTextMeshBuilder::Build(
	const TArray<FWorldTextItem>& Items,
	const FVector& Right,
	const FVector& Up,
	const FFontAtlas& Atlas,
	float WorldUnitsPerPixel)
{
	TArray<FVertexText> Vertices;
	for (int i = 0; i < Items.Num(); ++i)
	{
		AppendString(Vertices, Items[i].Text, Items[i].WorldPosition, Right, Up, Atlas, WorldUnitsPerPixel);
	}
	return Vertices;
}
