#include "pch.h"
#include "UCubeComponent.h"
#include "Engine/Resource/GResourceManager.h"
#include "Engine/Resource/FTextureResource.h"

void UCubeComponent::Initialize()
{
	Super::Initialize();
	Texture = GResourceManager::GetInstance()->GetOrLoadTexture("Assets/Textures/FlameTexture.png");
}

void UCubeComponent::SetTexture(const FString& FilePath)
{
	Texture = GResourceManager::GetInstance()->GetOrLoadTexture(FilePath);
}

FPrimitiveRenderData UCubeComponent::CreateRenderData(bool bSelected) const
{
	if (!Texture || !Texture->GetSRV()) return {};

	FPrimitiveRenderData OutData = Super::CreateRenderData(bSelected);

	OutData.Pipeline = EPrimitivePipeline::Texture;
	OutData.Material = Texture->GetSRV();
	OutData.UVTransform = FTextureUVTransform{ 1.0f, 1.0f, 0.0f, 0.0f };
	OutData.BlendMode = EPrimitiveBlendMode::Opaque;
	return OutData;
}