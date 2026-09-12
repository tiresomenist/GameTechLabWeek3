#pragma once

#include "Core/Container/TArray.h"
#include "Engine/Renderer/Text/FWorldTextItem.h"

class UScene;
class FEditor;
class UCameraComponent;
struct FPrimitiveRenderData;

namespace RenderUtil
{
	TArray<FPrimitiveRenderData> GetRenderList(FEditor* Editor, UScene* Scene);
	TArray<FPrimitiveRenderData> GetGizmoList(FEditor* Editor, UScene* Scene);
	TArray<FWorldTextItem> GetTextRenderList(UScene* Scene, const UCameraComponent* Camera, bool bShowUUIDWidgets);
};
