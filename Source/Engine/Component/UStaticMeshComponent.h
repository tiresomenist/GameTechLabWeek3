#pragma once
#include "Engine/Component/Primitive/UPrimitiveComponent.h"

class UStaticMeshComponent : public UPrimitiveComponent
{
    UCLASS(UStaticMeshComponent, "StaticMeshComponent", UPrimitiveComponent)

public:
    void SetStaticMesh(const FString& InMeshKey);
    virtual FMeshResource* GetMeshResource() const override;
    virtual void Serialize(FArchive& Archive) override;
    virtual void Deserialize(FArchive& Archive) override;

    const FString& GetStaticMeshKey() const { return MeshKey; }

private:
    FString MeshKey;

};
