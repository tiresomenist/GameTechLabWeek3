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
	FMeshResource* MeshResource = GResourceManager::GetInstance()->GetPrimitive(FString{ ClassType->Name });

	FPrimitiveRenderData OutData{};
	if (MeshResource == nullptr) { return OutData; }

	OutData.VertexBuffer = MeshResource->GetVertexBuffer();
	OutData.IndexBuffer = MeshResource->GetIndexBuffer();
	OutData.IndexCount = MeshResource->GetIndexCount();
	OutData.Stride = MeshResource->GetStride();
	OutData.WorldMatrix = &GetWorldMatrix();
	OutData.isSelected = bSelected;
	// RenderData.Material			= &GetMaterial();

	return OutData;
}