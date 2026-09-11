#pragma once

#include "Core/Container/TArray.h"

class UScene;
class FEditor;
struct FPrimitiveRenderData;
struct FTextDrawRequest;

namespace RenderUtil
{
	TArray<FPrimitiveRenderData> GetRenderList(FEditor* Editor, UScene* Scene);
	TArray<FPrimitiveRenderData> GetGizmoList(FEditor* Editor, UScene* Scene);
	TArray<FTextDrawRequest> GetUUIDTextRequests(UScene* Scene, float CharacterHeight, float GapRatio);
};