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

// 로컬 AABB의 8개 코너를 World로 옮겨서(=OBB) 12개 엣지를 배처에 넣는다.
void RenderUtil::PushBoundingBox(FLineBatcher& LineBatcher, const FVector& Min, const FVector& Max, const FMatrix& World)
{
	const FVector4 BoxColor(1.0f, 1.0f, 0.0f, 1.0f);

	// 인덱스의 비트 0/1/2 = X/Y/Z가 Min인지 Max인지
	FVertexSimple Corners[8];
	for (int i = 0; i < 8; ++i)
	{
		const FVector4 Local(
			(i & 1) ? Max.X : Min.X,
			(i & 2) ? Max.Y : Min.Y,
			(i & 4) ? Max.Z : Min.Z,
			1.0f);

		const FVector World3 = (Local * World).getXYZ();
		Corners[i] = { World3.X, World3.Y, World3.Z, BoxColor.X, BoxColor.Y, BoxColor.Z, BoxColor.W };
	}

	// 두 코너가 비트 하나만 다르면 그게 엣지 (정확히 12개)
	for (int i = 0; i < 8; ++i)
	{
		for (int Bit = 1; Bit <= 4; Bit <<= 1)
		{
			if (!(i & Bit))
			{
				LineBatcher.AddLine(Corners[i], Corners[i | Bit]);
			}
		}
	}
}
