#include "pch.h"
#include "RenderUtil.h"
#include "Core/Container/TArray.h"
#include "Engine/Renderer/FPrimitiveRenderData.h"
#include "Engine/Component/Primitive/UPrimitiveComponent.h"
#include "Engine/Component/Primitive/UTextComponent.h"
#include "Engine/Component/UWidgetComponent.h"
#include "Engine/Component/UCameraComponent.h"
#include "Engine/Resource/FMeshResource.h"
#include "Engine/Scene/UScene.h"
#include "Editor/FEditor.h"
#include "Editor/Gizmo/UGizmo.h"
#include "Engine/Renderer/Line/FLineDrawRequest.h"

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
	
	for (UGizmo* Item : Editor->GetGizmos())
	{
		TArray<FPrimitiveRenderData> Array = Item->GetRenderData();

		for (auto& Data : Array)
		{
			RenderList.Add(Data);
		}
	}
	return RenderList;
}

TArray<FWorldTextItem> RenderUtil::GetTextRenderList(UScene* Scene, const UCameraComponent* Camera, bool bShowUUIDWidgets)
{
	TArray<FWorldTextItem> TextList;
	if (!Scene || !Camera) return TextList;

	if (bShowUUIDWidgets)
	{
		Scene->ForEachWidget(
			[&TextList, Camera](UWidgetComponent* Widget)
			{
				FWorldTextItem Item;
				if (Widget->BuildTextItem(Camera, Item))
				{
					TextList.Add(Item);
				}
			}
		);
	}

	Scene->ForEachPrimitive(
		[&TextList, Camera](UPrimitiveComponent* Primitive)
		{
			if (Primitive->IsA(UTextComponent::GetClass()))
			{
				FWorldTextItem Item;
				auto* Text = static_cast<UTextComponent*>(Primitive);
				if (Text->BuildTextItem(Camera, Item))
				{
					TextList.Add(Item);
				}
			}
		}
	);
	return TextList;
}

void RenderUtil::AppendBoundsLineRequests(UScene* Scene, bool bShowAllBounds,
	const USceneComponent* SelectedComponent, TArray<FLineDrawRequest>& OutRequests)
{
	if (!Scene || (!bShowAllBounds && !SelectedComponent)) return;
	// 박스 하나를 월드 꼭짓점 8개와 모서리 인덱스 24개를 가진 요청으로 만든다.
	Scene->ForEachPrimitive([&](UPrimitiveComponent* Primitive) {
		// The selected primitive keeps its bounds even when the global toggle is off.
		if (!bShowAllBounds && Primitive != SelectedComponent) return;
		const FMeshResource* Mesh = Primitive->GetMeshResource();
		if (!Mesh || !Mesh->HasBounds()) return;
		const FVector Min = Mesh->GetBoundsMin();
		const FVector Max = Mesh->GetBoundsMax();
		const FMatrix& World = Primitive->GetWorldMatrix();
		FLineDrawRequest Request;
		Request.Points.SetNum(8);
		Request.B = 0.0f;

		for (uint32 i = 0; i < 8; ++i)
		{
			const FVector LocalCorner(
				(i & 1) ? Max.X : Min.X,
				(i & 2) ? Max.Y : Min.Y,
				(i & 4) ? Max.Z : Min.Z
			);

			Request.Points[i] = World.TransformPosition(LocalCorner);
		}

		Request.Indices = {
			0, 1, 0, 2, 1, 3, 2, 3,
			0, 4, 1, 5, 2, 6, 3, 7,
			4, 5, 4, 6, 5, 7, 6, 7
		};
		OutRequests.Add(Request);
		});
}
