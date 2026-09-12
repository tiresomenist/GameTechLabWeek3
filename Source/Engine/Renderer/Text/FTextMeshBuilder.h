#pragma once

#include "Core/Container/FString.h"
#include "Core/Container/TArray.h"
#include "Core/Math/FVector.h"
#include "Engine/Renderer/Text/FVertexText.h"
#include "Engine/Renderer/Text/FFontAtlas.h"
#include "Engine/Renderer/Text/FWorldTextItem.h"

// 문자열+월드위치 목록과 빌보드 축(Right/Up)을 받아
// 아틀라스에서 각 글자의 SubUV를 찾아 빌보드 quad 정점 목록을 만든다.
class FTextMeshBuilder
{
public:
	static TArray<FVertexText> Build(
		const TArray<FWorldTextItem>& Items,
		const FVector& Right,
		const FVector& Up,
		const FFontAtlas& Atlas,
		float WorldUnitsPerPixel = 0.02f);

private:
	static void AppendString(
		TArray<FVertexText>& OutVertices,
		const FString& Text,
		const FVector& WorldPosition,
		const FVector& Right,
		const FVector& Up,
		const FFontAtlas& Atlas,
		float WorldUnitsPerPixel);
};
