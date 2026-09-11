#include "pch.h"
#include "Engine/Renderer/RenderUtil.h"
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

TArray<FTextDrawRequest> RenderUtil::GetUUIDTextRequests(UScene* Scene, float CharacterHeight, float GapRatio)
{
    TArray<FTextDrawRequest> Requests;
    if (!Scene) return Requests;

    const float Gap = CharacterHeight * (std::max)(GapRatio, 0.0f);

    // BuildMesh는 전달받은 위치를 글자 중심으로 사용
    const float CenterOffset = CharacterHeight * 0.5f + Gap;

    Scene->ForEachPrimitive([&](UPrimitiveComponent* Primitive)
        {
            FVector Position = Primitive->GetWorldLocation();

            const FMeshResource* Mesh = Primitive->GetMeshResource();
            if (Mesh && Mesh->HasBounds())
            {
                const FVector Min = Mesh->GetBoundsMin();
                const FVector Max = Mesh->GetBoundsMax();
                const FMatrix& World = Primitive->GetWorldMatrix();

                // 변환된 박스의 중심: 라벨의 가로 위치에 사용
                Position = World.TransformPosition((Min + Max) * 0.5f);

                float TopZ = std::numeric_limits<float>::lowest();

                // 바운딩박스 사용해 위쪽 계산
                for (int Corner = 0; Corner < 8; ++Corner)
                {
                    //비트연산으로 정함
                    const FVector LocalCorner((Corner & 1) ? Max.X : Min.X,(Corner & 2) ? Max.Y : Min.Y,(Corner & 4) ? Max.Z : Min.Z);

                    const FVector WorldCorner = World.TransformPosition(LocalCorner);

                    TopZ = (std::max)(TopZ, WorldCorner.Z);
                }

                Position.Z = TopZ;
            }

            Position.Z += CenterOffset;

            Requests.Add({
                std::format("{}", Primitive->GetUUID()),
                Position,
                CharacterHeight
                });
        });

    return Requests;
}