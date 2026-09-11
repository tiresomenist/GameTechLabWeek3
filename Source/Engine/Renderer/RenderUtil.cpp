#include "pch.h"
#include "RenderUtil.h"
#include "Core/Container/TArray.h"
#include "Engine/Renderer/FPrimitiveRenderData.h"
#include "Engine/Component/Primitive/UPrimitiveComponent.h"
#include "Engine/Resource/FMeshResource.h"
#include "Engine/Scene/UScene.h"
#include "Editor/FEditor.h"
#include "Editor/Gizmo/UGizmo.h"
#include "Editor/UGrid.h"

#include <string>

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
			RenderList.Add(Primitive->CreateRenderData(bSelected));
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

TArray<FWorldTextItem> RenderUtil::GetTextRenderList(UScene* Scene)
{
	TArray<FWorldTextItem> TextList;
	Scene->ForEachPrimitive(
		[&TextList](UPrimitiveComponent* Primitive)
		{
			const FVector Base = Primitive->GetWorldLocation();

			// 오브젝트 바운즈 상단 위로 라벨을 띄운다. 바운즈가 없으면 고정치로 대체.
			float ZOffset = 1.5f;
			if (FMeshResource* Mesh = Primitive->GetMeshResource(); Mesh && Mesh->HasBounds())
			{
				ZOffset = Mesh->GetBoundsMax().Z * Primitive->GetRelativeScale3D().Z + 0.3f;
			}

			TextList.Add(FWorldTextItem{
				std::to_string(Primitive->GetUUID()),
				Base + FVector(0.0f, 0.0f, ZOffset)
			});
		}
	);
	return TextList;
}
