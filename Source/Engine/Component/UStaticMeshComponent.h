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

    void SetMaterial(const FString& TexturePath);
    void SetMaterial(FTextureResource* InTexture);
    FPrimitiveRenderData CreateRenderData(bool bSelected = false) const override;

    const FString& GetStaticMeshKey() const { return MeshKey; }
    const FString& GetMaterialPath() const { return MaterialPath; }

    bool IsVisible() { return bIsVisible; }
    void SetVisibility(bool InVisibility) { bIsVisible = InVisibility; }
private:
    bool bIsVisible = true;
    FString MeshKey;
    FString MaterialPath;
    FTextureResource* MaterialTexture = nullptr;
};
