#pragma once

#include "Core/Container/FString.h"
#include "Core/Math/FVector.h"

// 월드 공간에 표시할 텍스트 라벨 하나 (문자열 + 표시 위치)
struct FWorldTextItem
{
	FString Text;
	FVector WorldPosition;
};
