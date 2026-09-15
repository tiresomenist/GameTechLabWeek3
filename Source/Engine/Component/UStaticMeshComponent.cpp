#include "pch.h"
#include "UStaticMeshComponent.h"
#include "Engine/Object/FArchive.h"

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
}

void UStaticMeshComponent::Deserialize(FArchive& Archive)
{
    Super::Deserialize(Archive);
    SetStaticMesh(Archive.Contains("MeshKey")
        ? Archive.GetString("MeshKey")
        : FString{});
}
