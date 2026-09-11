#pragma once

#include "Container/FString.h"
#include "Engine/Object/USceneComponent.h"
#include "Engine/Renderer/FPrimitiveRenderData.h"
#include "../../GResourceManager.h"
struct FMeshResource;

class UPrimitiveComponent : public USceneComponent
{

    UCLASS(UPrimitiveComponent, "PrimitiveComponent", USceneComponent)

public:

    virtual void Initialize() override;

    virtual FPrimitiveRenderData CreateRenderData(bool bSelected = false) const;

    FMeshResource* GetMeshResource() const
    {
        return GResourceManager::GetInstance()->GetPrimitive(FString(GetInstanceClass()->Name));
    }
};

