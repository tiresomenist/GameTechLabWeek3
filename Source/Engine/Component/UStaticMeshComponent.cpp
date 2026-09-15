#include "pch.h"
#include "UStaticMeshComponent.h"
#include "Engine/Object/FArchive.h"
#include "Engine/Resource/FTextureResource.h"

void UStaticMeshComponent::SetStaticMesh(const FString& InMeshKey)
{
    MeshKey = InMeshKey;
}

FMeshResource* UStaticMeshComponent::GetMeshResource() const
{
    return GResourceManager::GetInstance()->GetPrimitive(MeshKey);
}

void UStaticMeshComponent::Serialize(FArchive& Archive)
{
    Super::Serialize(Archive);
    Archive.SetString("MeshKey", MeshKey);
    Archive.SetString("MaterialPath", MaterialPath);
}

void UStaticMeshComponent::Deserialize(FArchive& Archive)
{
    Super::Deserialize(Archive);
    SetStaticMesh(Archive.Contains("MeshKey")
        ? Archive.GetString("MeshKey")
        : FString{});
    if (Archive.Contains("MaterialPath"))
        SetMaterial(Archive.GetString("MaterialPath"));
}


void UStaticMeshComponent::SetMaterial(const FString& TexturePath)
{
    if (TexturePath.empty())
    {
        MaterialTexture = nullptr;
        MaterialPath.clear();
        return;
    }
    try
    {
        MaterialTexture = GResourceManager::GetInstance()->GetOrLoadTexture(TexturePath);
            MaterialPath = TexturePath;
    }
    catch (const std::exception& e)
    { 
        //UE_LOG("텍스처 로딩 실패 (%s): %s", TexturePath.c_str(), e.what());
    }
}
void UStaticMeshComponent::SetMaterial(FTextureResource* InTexture)
{
    MaterialTexture = InTexture;
}
FPrimitiveRenderData UStaticMeshComponent::CreateRenderData(bool bSelected) const
{
    FPrimitiveRenderData OutData = Super::CreateRenderData(bSelected);
    if (OutData.VertexBuffer == nullptr) return OutData;

    if (MaterialTexture && MaterialTexture->GetSRV())
    {
        OutData.Pipeline = EPrimitivePipeline::Texture;
        OutData.Material = MaterialTexture->GetSRV();
        OutData.UVTransform = FTextureUVTransform{ 1.0f, 1.0f, 0.0f, 0.0f };
        OutData.BlendMode = EPrimitiveBlendMode::Opaque;
    }
    return OutData;
}