#pragma once

#include "Core/Container/FString.h"
#include "Core/Container/TArray.h"
#include "Core/Math/FVector.h"
#include "Engine/Renderer/Text/FFontAtlas.h"
#include "Engine/Renderer/Text/FWorldTextItem.h"
#include "Engine/Renderer/FVertexSimple.h"

// 문자열과 각각의 월드 행렬을 받아 아틀라스 글리프 quad 정점 목록을 만든다.
class FTextMeshBuilder
{
public:
	static TArray<FVertexTexture> Build(
		const TArray<FWorldTextItem>& Items,
		const FFontAtlas& Atlas,
		float WorldUnitsPerPixel = 0.02f);

	// Build와 같은 글리프 레이아웃으로 로컬 공간 Bounds를 계산한다.
	static bool GetLocalBounds(
		const FString& Text,
		const FFontAtlas& Atlas,
		FVector& OutMin,
		FVector& OutMax,
		float WorldUnitsPerPixel = 0.02f);

private:
	static void AppendString(
		TArray<FVertexTexture>& OutVertices,
		const FString& Text,
		const FMatrix& WorldMatrix,
		const FFontAtlas& Atlas,
		float WorldUnitsPerPixel);
};
