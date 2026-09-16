#include "pch.h"
#include "RenderUtil.h"
#include "Core/Container/TArray.h"
#include "Engine/Renderer/FPrimitiveRenderData.h"
#include "Engine/Component/Primitive/UPrimitiveComponent.h"
#include "Engine/Component/Primitive/UTextComponent.h"
#include "Engine/Component/UWidgetComponent.h"
#include "Engine/Component/UCameraComponent.h"
#include "Engine/Component/UStaticMeshComponent.h"
#include "Engine/Resource/FMeshResource.h"
#include "Engine/Scene/UScene.h"
#include "Editor/FEditor.h"
#include "Editor/Gizmo/UGizmo.h"
#include "Editor/UGrid.h"
#include "Engine/Actor/AActor.h"
#include "Engine/Component/Light/USpotLightComponent.h"
#include <string>
#include "Engine/Log.h"


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
	if (!Editor || !Scene) return RenderList;

	const UCameraComponent* Camera = Editor->GetEditorCamera();
	Scene->ForEachPrimitive(
		[&RenderList, Editor, Camera](UPrimitiveComponent* Primitive)
		{
			if (Primitive->IsA(UStaticMeshComponent::GetClass()))
			{
				auto* StaticMesh = static_cast<UStaticMeshComponent*>(Primitive);
				if (!StaticMesh->IsVisible())
				{
					return;
				}
			}

			const bool bSelected = IsComponentSelected(Editor, Primitive);
			FPrimitiveRenderData Data = Primitive->CreateRenderData(bSelected);

			// 현재 카메라 기준의 렌더링용 행렬 연결함
			Data.WorldMatrix = &Primitive->GetRenderWorldMatrix(Camera);
			RenderList.Add(Data);
		}
	);

	//SpotLight를 렌더링하기위한 임시 순회, 차후에 분리 해야함.
	Scene->ForEachActor([&](AActor* Actor)
		{
			for (UActorComponent* Component : Actor->GetComponents())
			{
				if (!Component->IsA(USpotLightComponent::GetClass())){continue;}

				const auto* SpotLight = static_cast<const USpotLightComponent*>(Component);

				const bool bSelected = IsComponentSelected(Editor, SpotLight);

				FPrimitiveRenderData Data =	SpotLight->BuildIconRenderData(Camera, bSelected);

				// 유효한 렌더 데이터만 목록에 추가함
				if (Data.VertexBuffer && Data.IndexBuffer && Data.Material && Data.WorldMatrix && Data.IndexCount > 0)
				{
					RenderList.Add(Data);
				}
			}
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

void RenderUtil::SubmitLineDrawRequests(FEditor* Editor,UScene* Scene,FLineBatcher& Batcher)
{
	if (!Editor || !Scene) { return; }

	const UCameraComponent* Camera = Editor->GetEditorCamera();

	if (!Camera) { return; }

	// 요청을 별도로 저장하지 않고 배처의 통합 배열에 즉시 병합함
	const FLineRequestConsumer Submit =	[&Batcher](const FLineDrawRequest& Request)
		{
			if (!Batcher.AddRequest(Request))
			{
				UE_LOG("[RenderUtil] 잘못되었거나 용량을 초과한 라인 요청");
			}
		};

	FLineDrawContext Context;
	Context.Camera = Camera;
	Context.bShowBounds = Editor->IsShowingBoundingBoxes();
	Context.bShowPrimitives = Editor->IsShowingPrimitives();

	// 각 프리미티브가 생성한 바운딩 박스 요청을 즉시 제출함
	Scene->ForEachPrimitive([&](UPrimitiveComponent* Primitive)
		{
			Context.bSelected =	Editor->GetSelectedSceneComponent() == Primitive;
			Primitive->SubmitLineDrawRequests(Context, Submit);
		}
	);

	// 추후 에디터-게임씬 분리시 에디터 단으로 이동해야함
	if (Context.bShowPrimitives)
	{
		const USceneComponent* Selected =
			Editor->GetSelectedSceneComponent();

		if (Selected && Selected->IsA(USpotLightComponent::GetClass()))
		{
			const auto* SpotLight =
				static_cast<const USpotLightComponent*>(Selected);

			// 생성한 원뿔 요청을 배처의 통합 배열에 즉시 병합함
			Submit(SpotLight->BuildConeLineDrawRequest());
		}
	}

	const FVector CameraPosition = Camera->GetWorldLocation();
	const FGrid& GridSettings = Editor->GetGrid();

	// 생성된 그리드 요청을 즉시 제출함
	if (Editor->IsShowingGrid())
	{
		for (UGrid* Grid : Editor->GetGrids())
		{
			Submit(Grid->BuildLineDrawRequest(GridSettings,CameraPosition));
		}
	}

	// 생성된 기즈모 라인 요청을 즉시 제출함
	if (Editor->IsShowingWorldAxis()) {
		for (UGizmo* Gizmo : Editor->GetGizmos())
		{
			Submit(Gizmo->BuildLineDrawRequest(GridSettings, CameraPosition));
		}
	}
}