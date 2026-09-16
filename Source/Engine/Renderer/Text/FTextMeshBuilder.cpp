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
	TArray<FVertexTexture>& OutVertices,
	const FString& Text,
	const FMatrix& WorldMatrix,
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

		// 로컬 +Y(가로), +Z(세로) 좌표를 각 Item의 빌보드 행렬로 월드 공간에 변환한다.
		auto ToWorld = [&](float lx, float ly)
		{
			return WorldMatrix.TransformPosition(FVector(0.0f, lx * WorldUnitsPerPixel, ly * WorldUnitsPerPixel));
		};

		const FVector P0 = ToWorld(x0, y0); // 좌상
		const FVector P1 = ToWorld(x1, y0); // 우상
		const FVector P2 = ToWorld(x1, y1); // 우하
		const FVector P3 = ToWorld(x0, y1); // 좌하

		auto PushVertex = [&](const FVector& P, float U, float V)
		{
			FVertexTexture Vert;
			Vert.x = P.X; Vert.y = P.Y; Vert.z = P.Z;
			Vert.u = U; Vert.v = V;
			Vert.r = 1.0f; Vert.g = 1.0f; Vert.b = 1.0f; Vert.a = 1.0f;
			OutVertices.Add(Vert);
		};

		PushVertex(P0, Glyph->U0, Glyph->V0);
		PushVertex(P1, Glyph->U1, Glyph->V0);
		PushVertex(P2, Glyph->U1, Glyph->V1);
		PushVertex(P3, Glyph->U0, Glyph->V1);

		PenX += Glyph->XAdvance;
	}
}

TArray<FVertexTexture> FTextMeshBuilder::Build(
	const TArray<FWorldTextItem>& Items,
	const FFontAtlas& Atlas,
	float WorldUnitsPerPixel)
{
	TArray<FVertexTexture> Vertices;
	for (int i = 0; i < Items.Num(); ++i)
	{
		AppendString(Vertices, Items[i].Text, Items[i].WorldMatrix, Atlas, WorldUnitsPerPixel);
	}
	return Vertices;
}

bool FTextMeshBuilder::GetLocalBounds(
	const FString& Text,
	const FFontAtlas& Atlas,
	FVector& OutMin,
	FVector& OutMax,
	float WorldUnitsPerPixel)
{
	const TArray<char32_t> Codepoints = DecodeUTF8(Text);
	float TotalWidth = 0.0f;
	for (int i = 0; i < Codepoints.Num(); ++i)
	{
		if (const FGlyphInfo* Glyph = Atlas.FindGlyph(Codepoints[i]))
			TotalWidth += Glyph->XAdvance;
	}

	float PenX = -TotalWidth * 0.5f;
	bool bHasGlyphBounds = false;
	for (int i = 0; i < Codepoints.Num(); ++i)
	{
		const FGlyphInfo* Glyph = Atlas.FindGlyph(Codepoints[i]);
		if (!Glyph)
			continue;

		const float X0 = PenX + Glyph->XOffset;
		const float X1 = X0 + Glyph->Width;
		const float Y0 = -Glyph->YOffset;
		const float Y1 = Y0 - Glyph->Height;
		const FVector GlyphMin(0.0f, (std::min)(X0, X1) * WorldUnitsPerPixel, (std::min)(Y0, Y1) * WorldUnitsPerPixel);
		const FVector GlyphMax(0.0f, (std::max)(X0, X1) * WorldUnitsPerPixel, (std::max)(Y0, Y1) * WorldUnitsPerPixel);

		if (!bHasGlyphBounds)
		{
			OutMin = GlyphMin;
			OutMax = GlyphMax;
			bHasGlyphBounds = true;
		}
		else
		{
			OutMin.X = (std::min)(OutMin.X, GlyphMin.X);
			OutMin.Y = (std::min)(OutMin.Y, GlyphMin.Y);
			OutMin.Z = (std::min)(OutMin.Z, GlyphMin.Z);
			OutMax.X = (std::max)(OutMax.X, GlyphMax.X);
			OutMax.Y = (std::max)(OutMax.Y, GlyphMax.Y);
			OutMax.Z = (std::max)(OutMax.Z, GlyphMax.Z);
		}

		PenX += Glyph->XAdvance;
	}

	return bHasGlyphBounds;
}
