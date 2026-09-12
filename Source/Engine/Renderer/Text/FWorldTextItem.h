#pragma once

#include "Core/Container/FString.h"
#include "Core/Math/Matrix.h"

// 월드 공간에 표시할 텍스트 라벨 하나.
// 텍스트 정점은 로컬 +Y(가로), +Z(세로) 평면에서 만든 뒤 WorldMatrix로 변환한다.
struct FWorldTextItem
{
	FString Text;
	FMatrix WorldMatrix;
};
