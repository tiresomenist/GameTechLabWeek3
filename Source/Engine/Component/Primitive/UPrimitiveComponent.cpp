#include "pch.h"
#include "UPrimitiveComponent.h"
#include "Engine/Resource/GResourceManager.h"
#include "Engine/Resource/FMeshResource.h"

void UPrimitiveComponent::Initialize()
{
	Super::Initialize();
}

FPrimitiveRenderData UPrimitiveComponent::CreateRenderData(bool bSelected) const
{
	FClassType* ClassType = GetInstanceClass();
	FMeshResource* MeshResource = GetMeshResource();

	FPrimitiveRenderData OutData{};
	if (MeshResource == nullptr) { return OutData; }

	OutData.VertexBuffer = MeshResource->GetVertexBuffer();
	OutData.IndexBuffer = MeshResource->GetIndexBuffer();
	OutData.IndexCount = MeshResource->GetIndexCount();
	OutData.Stride = MeshResource->GetStride();
	OutData.WorldMatrix = &GetWorldMatrix();
	OutData.isSelected = bSelected;
	// RenderData.Material			= &GetMaterial();
	OutData.Min = MeshResource->GetBoundsMin();
	OutData.Max = MeshResource->GetBoundsMax();

	return OutData;
}

bool UPrimitiveComponent::GetLocalBounds(FVector& OutMin, FVector& OutMax) const
{
	// 기본: 클래스 이름으로 찾은 공유 메시의 Bounds
	FMeshResource* MeshResource = GetMeshResource();
	if (!MeshResource || !MeshResource->HasBounds()) return false;

	OutMin = MeshResource->GetBoundsMin();
	OutMax = MeshResource->GetBoundsMax();
	return true;
}

const FMatrix& UPrimitiveComponent::GetRenderWorldMatrix(const UCameraComponent* Camera) const
{
	return GetWorldMatrix();
}

FMeshResource* UPrimitiveComponent::GetMeshResource() const
{
    const FName MeshName(GetInstanceClass()->Name);
    return GResourceManager::GetInstance()->GetPrimitive(MeshName);
}

void UPrimitiveComponent::SubmitLineDrawRequests(const FLineDrawContext& Context, const FLineRequestConsumer& Submit) const
{
    if (!Context.bShowBounds && !Context.bSelected)
    {
        return;
    }

    FVector Min;
    FVector Max;

    if (!GetLocalBounds(Min, Max))
    {
        return;
    }

    FLineDrawRequest Request;
    Request.Vertices.Reserve(8);
    Request.Indices.Reserve(24);

    // 빌보드 오브젝트도 실제 표시되는 자세에 맞춰 박스를 생성함
    const FMatrix& World = GetRenderWorldMatrix(Context.Camera);

    for (uint32 Corner = 0; Corner < 8; ++Corner)
    {
        const FVector Local(
            (Corner & 1) ? Max.X : Min.X,
            (Corner & 2) ? Max.Y : Min.Y,
            (Corner & 4) ? Max.Z : Min.Z);

        const FVector Position = World.TransformPosition(Local);

        Request.Vertices.Add({ Position.X, Position.Y, Position.Z,1.0f, 1.0f, 0.0f, 1.0f });
    }

    // 한 축의 비트만 다른 두 꼭짓점을 연결함
    for (uint32 Corner = 0; Corner < 8; ++Corner)
    {
        for (uint32 Bit = 1; Bit <= 4; Bit <<= 1)
        {
            if ((Corner & Bit) == 0)
            {
                Request.Indices.Add(Corner);
                Request.Indices.Add(Corner | Bit);
            }
        }
    }
    Submit(Request);
}
