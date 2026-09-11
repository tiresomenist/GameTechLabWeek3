#pragma once

#include "Engine/Component/UCameraComponent.h"
#include "Engine/Resource/GResourceManager.h"
#include "Editor/Gizmo/UGizmo.h"

class UScene;
class FEditor;

struct FRay;

class FGizmoPicker
{
public:
	FGizmoPicker(FEditor* InEditor);
	~FGizmoPicker();
	bool RayTriangleIntersect(const FRay& Ray, FVector A, FVector B, FVector C, float& OutDistance);
	bool MakeWorldRay(FRay& OutRay);
	int Pick(UGizmo* InGizmos);
private:
	FEditor* Editor;
};
