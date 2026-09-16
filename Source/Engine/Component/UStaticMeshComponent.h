#pragma once
#include "Engine/Component/Primitive/UPrimitiveComponent.h"
#include "Core/Name/FName.h"

class UStaticMeshComponent : public UPrimitiveComponent
{
    UCLASS(UStaticMeshComponent, "StaticMeshComponent", UPrimitiveComponent)

public:
    void SetStaticMesh(const FName& InMeshKey);
    virtual FMeshResource* GetMeshResource() const override;
    virtual void Serialize(FArchive& Archive) override;
    virtual void Deserialize(FArchive& Archive) override;

    void SetMaterial(const FString& TexturePath);
    void SetMaterial(FTextureResource* InTexture);
    FPrimitiveRenderData CreateRenderData(bool bSelected = false) const override;

    const FName& GetStaticMeshKey() const { return MeshKey; }
    const FString& GetMaterialPath() const { return MaterialPath; }

    bool IsVisible() { return bIsVisible; }
    void SetVisibility(bool InVisibility) { bIsVisible = InVisibility; }
private:
    bool bIsVisible = true;
    FName MeshKey;
    FString MaterialPath;
    FTextureResource* MaterialTexture = nullptr;
};
