#pragma once

#include "Core/Math/FVector.h"

class UScene;
class FEditor;
class UPrimitiveComponent;
class USceneComponent;

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
	bool RayAABBIntersect(const FRay& Ray, const FVector& BoundsMin, const FVector& BoundsMax, float MaxDistance, float& OutDistance);
	USceneComponent* Pick();

private:
	FEditor* Editor;
};


