#include "pch.h"
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

	RenderData.VertexBuffer = MeshResource->VertexBuffer;
	RenderData.IndexBuffer = MeshResource->IndexBuffer;
	RenderData.IndexCount = MeshResource->IndexCount;
	RenderData.Stride = MeshResource->Stride;
	// RenderData.Material			= &GetMaterial();

	return RenderData;
}