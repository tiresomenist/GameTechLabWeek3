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
	auto* SelectedComponent = Editor ? Editor->GetSelectedSceneComponent() : nullptr;

	for (auto Item : Scene->Objects)
	{
		if (Item->IsA(UPrimitiveComponent::GetClass()))
		{
			UPrimitiveComponent* Primitive = static_cast<UPrimitiveComponent*>(Item);
			FPrimitiveRenderData RenderData = Primitive->CreateRenderData();
			
			if (SelectedComponent && Primitive == SelectedComponent)
			{
				RenderData.isSelected = true;
			}

			RenderList.Add(RenderData);
		}
	}

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