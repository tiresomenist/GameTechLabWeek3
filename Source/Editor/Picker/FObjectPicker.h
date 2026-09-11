#pragma once

#include "Core/Math/FVector.h"

class UScene;
class FEditor;
class UPrimitiveComponent;

struct FRay {
	FVector Origin;
	FVector Direction;
};

class FObjectPicker
{
public:
	FObjectPicker(FEditor* InEditor);
	bool MakeWorldRay(FRay& OutRay);
	bool RayTriangleIntersect(const FRay& Ray, FVector A, FVector B, FVector C, float& OutDistance);
	// OutEnter: 광선이 상자에 들어가는 거리 (두께 0인 평면이면 평면에 닿는 거리)
	bool RayAABBIntersect(const FRay& Ray, const FVector& BoundsMin, const FVector& BoundsMax, float MaxDistance, float* OutEnter = nullptr);
	UPrimitiveComponent* Pick();

private:
	FEditor* Editor;
};


