#pragma once

#include "Engine/Component/Primitive/UPrimitiveComponent.h"
#include "Engine/Resource/GResourceManager.h"
#include "Engine/Object/FClassType.h"

class FTextureResource;

class UCubeComponent : public UPrimitiveComponent
{
    UCLASS(UCubeComponent, "Cube", UPrimitiveComponent)

public:
    virtual void Initialize() override;
    virtual FPrimitiveRenderData CreateRenderData(bool bSelected = false) const override;

    void SetTexture(FTextureResource* InTexture) { Texture = InTexture; }
    void SetTexture(const FString& FilePath);
    FTextureResource* GetTexture() const { return Texture; }

private:
    FTextureResource* Texture = nullptr;
};