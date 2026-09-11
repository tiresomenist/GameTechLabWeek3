#include "pch.h"
#include "UGizmo.h"
#include "Engine/Object/UObject.h"
#include "Engine/Object/FClassType.h"
#include "Engine/Renderer/FVertexSimple.h"

void UGizmo::Initialize(FEditor* InEditor)
{
	Editor = InEditor;
}

TArray<FPrimitiveRenderData> UGizmo::GetRenderData()
{
	return TArray<FPrimitiveRenderData>();
}