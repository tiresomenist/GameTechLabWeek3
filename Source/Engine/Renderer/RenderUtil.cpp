#include "pch.h"
#include "RenderUtil.h"
#include "Core/Container/TArray.h"
#include "Engine/Renderer/FPrimitiveRenderData.h"
#include "Engine/Component/Primitive/UPrimitiveComponent.h"
#include "Engine/Scene/UScene.h"
#include "Editor/FEditor.h"
#include "Editor/Gizmo/UGizmo.h"
#include "Editor/UGrid.h"

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
	Scene->ForEachPrimitive(
		[&RenderList, Editor](UPrimitiveComponent* Primitive)
		{
			const bool bSelected = IsComponentSelected(Editor, Primitive);
			FPrimitiveRenderData Data = Primitive->CreateRenderData(bSelected);

			// 그릴 메시가 없는 컴포넌트(메시를 못 찾음, 빈 텍스트 등)는 목록에서 뺀다.
			// 렌더러는 WorldMatrix를 바로 역참조하므로 비어 있는 데이터가 들어가면 크래시가 난다.
			if (!Data.VertexBuffer || !Data.IndexBuffer || Data.IndexCount == 0 || !Data.WorldMatrix) return;

			RenderList.Add(Data);
		}
	);

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
