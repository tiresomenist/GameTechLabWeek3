#include "pch.h"
#include "UGrid.h"
#include "../../Engine/Object/UObject.h"
#include "../../Engine/Object/FClassType.h"
#include "../../Engine/Renderer/FVertexSimple.h"
#include "Engine/GResourceManager.h"

void UGrid::Initialize(FEditor* InEditor)
{
	Editor = InEditor;
	MeshResource = GResourceManager::GetInstance()->GetPrimitive("Grid");
}

TArray<FPrimitiveRenderData> UGrid::GetRenderData()
{
	return TArray<FPrimitiveRenderData>();
}