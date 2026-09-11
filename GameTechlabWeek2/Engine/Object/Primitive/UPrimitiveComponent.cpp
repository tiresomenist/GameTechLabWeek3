#include "UPrimitiveComponent.h"
#include "Engine/GResourceManager.h"
#include "Engine/Primitive/FMeshResource.h"

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

	OutData.VertexBuffer = MeshResource->VertexBuffer;
	OutData.IndexBuffer = MeshResource->IndexBuffer;
	OutData.IndexCount = MeshResource->IndexCount;
	OutData.Stride = MeshResource->Stride;
	OutData.WorldMatrix = &GetWorldMatrix();
	OutData.isSelected = bSelected;
	// RenderData.Material			= &GetMaterial();

	return OutData;
}