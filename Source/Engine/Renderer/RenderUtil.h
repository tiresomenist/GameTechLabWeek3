#pragma once

#include "Core/Container/TArray.h"
#include "Engine/Renderer/Text/FWorldTextItem.h"
#include "Engine/Renderer/Line/FLineBatcher.h"
#include "Engine/Renderer/Line/FLineDrawRequest.h"
class UScene;
class FEditor;
class UCameraComponent;
struct FPrimitiveRenderData;

namespace RenderUtil
{
	TArray<FPrimitiveRenderData> GetRenderList(FEditor* Editor, UScene* Scene);
	TArray<FPrimitiveRenderData> GetGizmoList(FEditor* Editor, UScene* Scene);
	TArray<FWorldTextItem> GetTextRenderList(UScene* Scene, const UCameraComponent* Camera, bool bShowUUIDWidgets);
	void SubmitLineDrawRequests(FEditor* Editor,UScene* Scene,FLineBatcher& Batcher);
};
