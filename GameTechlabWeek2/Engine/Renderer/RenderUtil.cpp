#include "RenderUtil.h"
#include "Container/TArray.h"
#include "Engine/Renderer/FPrimitiveRenderData.h"
#include "Engine/Object/Primitive/UPrimitiveComponent.h"
#include "Engine/Scene/UScene.h"
#include "Engine/Editor/FEditor.h"
#include "Engine/Gizmo/UGizmo.h"
#include "Engine/Editor/UGrid.h"

namespace
{
	bool IsComponentSelected(const FEditor* Editor, const USceneComponent* Component)
	{
		if (!Editor || !Component) return false;
		return Editor->GetSelectedSceneComponent() == Component;
	}
}

TArray<FPrimitiveRenderData> RenderUtil::GetRenderList(FEditor* Editor, UScene* Scene)
{
	TArray<FPrimitiveRenderData> RenderList;
	auto* SelectedComponent = Editor ? Editor->GetSelectedSceneComponent() : nullptr;

	for (auto Item : Scene->Objects)
	{
		if (Item->IsA(UPrimitiveComponent::GetClass()))
		{
			UPrimitiveComponent* Primitive = static_cast<UPrimitiveComponent*>(Item);
			const bool bSelected = IsComponentSelected(Editor, Primitive);
			RenderList.Add(Primitive->CreateRenderData(bSelected));
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