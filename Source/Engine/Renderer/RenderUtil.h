#pragma once

#include "Core/Container/TArray.h"
#include "Engine/Renderer/Text/FWorldTextItem.h"
#include "FLineBatcher.h"

class UScene;
class FEditor;
class UCameraComponent;
struct FPrimitiveRenderData;

namespace RenderUtil
{
	TArray<FPrimitiveRenderData> GetRenderList(FEditor* Editor, UScene* Scene);
	TArray<FPrimitiveRenderData> GetGizmoList(FEditor* Editor, UScene* Scene);
	TArray<FWorldTextItem> GetTextRenderList(UScene* Scene, const UCameraComponent* Camera, bool bShowUUIDWidgets);
	void PushBoundingBox(FLineBatcher& LineBatcher, const FVector& Min, const FVector& Max, const FMatrix& World);
};
