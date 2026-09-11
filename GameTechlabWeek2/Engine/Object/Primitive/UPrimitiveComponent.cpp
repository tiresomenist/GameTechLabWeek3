#include "UPrimitiveComponent.h"
#include "Engine/GResourceManager.h"
#include "Engine/Primitive/FMeshResource.h"

void UPrimitiveComponent::Initialize()
{
	Super::Initialize();

	FClassType* ClassType = GetInstanceClass();
	RenderData = CreateRenderData(ClassType->Name);
	RenderData.WorldMatrix = &GetWorldMatrix();
}

FPrimitiveRenderData UPrimitiveComponent::CreateRenderData(FStringView Type)
{
	GResourceManager& ResourceManager = *GResourceManager::GetInstance();
	FMeshResource* MeshResource = ResourceManager.GetPrimitive(FString{ Type });

	FPrimitiveRenderData RenderData{};
	if (MeshResource == nullptr) { return RenderData; }

	RenderData.VertexBuffer = MeshResource->GetVertexBuffer();
	RenderData.IndexBuffer = MeshResource->GetIndexBuffer();
	RenderData.IndexCount = MeshResource->GetIndexCount();
	RenderData.Stride = MeshResource->GetStride();
	// RenderData.Material			= &GetMaterial();

	return RenderData;
}