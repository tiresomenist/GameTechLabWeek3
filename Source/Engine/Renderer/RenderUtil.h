#pragma once

#include "Core/Container/TArray.h"
#include "Engine/Renderer/Text/FWorldTextItem.h"

class UScene;
class FEditor;
class UCameraComponent;
class USceneComponent;
struct FPrimitiveRenderData;
struct FLineDrawRequest;

namespace RenderUtil
{
	TArray<FPrimitiveRenderData> GetRenderList(FEditor* Editor, UScene* Scene);
	TArray<FPrimitiveRenderData> GetGizmoList(FEditor* Editor, UScene* Scene);
	TArray<FWorldTextItem> GetTextRenderList(UScene* Scene, const UCameraComponent* Camera, bool bShowUUIDWidgets);
	void AppendBoundsLineRequests(UScene* Scene, bool bShowAllBounds,
		const USceneComponent* SelectedComponent, TArray<FLineDrawRequest>& OutRequests);
};
