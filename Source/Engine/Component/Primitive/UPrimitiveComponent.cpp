#include "pch.h"
#include "UPrimitiveComponent.h"
#include "Engine/Resource/GResourceManager.h"
#include "Engine/Resource/FMeshResource.h"
#include "Engine/Resource/FTextureResource.h"

void UPrimitiveComponent::Initialize()
{
	Super::Initialize();
}

FPrimitiveRenderData UPrimitiveComponent::CreateRenderData(bool bSelected) const
{
	FMeshResource* MeshResource = GetMeshResource();

	FPrimitiveRenderData OutData{};
	if (MeshResource == nullptr) { return OutData; }

	OutData.VertexBuffer = MeshResource->GetVertexBuffer();
	OutData.IndexBuffer = MeshResource->GetIndexBuffer();
	OutData.IndexCount = MeshResource->GetIndexCount();
	OutData.Stride = MeshResource->GetStride();
	OutData.Material = Material ? Material->GetShaderResourceView() : nullptr;
	OutData.BlendMode = BlendMode;
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
