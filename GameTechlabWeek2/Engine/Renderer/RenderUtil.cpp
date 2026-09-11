#include "RenderUtil.h"
#include "Container/TArray.h"
#include "Engine/Renderer/FPrimitiveRenderData.h"
#include "Engine/Object/Primitive/UPrimitiveComponent.h"
#include "Engine/Scene/UScene.h"
#include "Engine/Editor/FEditor.h"
#include "Engine/Gizmo/UGizmo.h"
#include "Engine/Editor/UGrid.h"

TArray<FPrimitiveRenderData> RenderUtil::GetRenderList(FEditor* Editor, UScene* Scene)
{
	TArray<FPrimitiveRenderData> RenderList;

	Scene->ForEachPrimitive(
		[&RenderList](UPrimitiveComponent* Primitive)
		{
			RenderList.Add(Primitive->GetRenderData());
		});

	return RenderList;
}

TArray<FPrimitiveRenderData> RenderUtil::GetGizmoList(FEditor* Editor, UScene* Scene)
{
	TArray<FPrimitiveRenderData> RenderList;
	
	for (auto Item : Editor->Gizmos)
	{
		TArray<FPrimitiveRenderData> Array = Item->GetRenderData();

		for (auto& Data : Array)
		{
			RenderList.Add(Data);
		}
	}
	return RenderList;
}
