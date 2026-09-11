#pragma once

#include "Core/Container/FString.h"
#include "Engine/Component/USceneComponent.h"
#include "Engine/Renderer/FPrimitiveRenderData.h"
#include "Engine/Resource/GResourceManager.h"
struct FMeshResource;

class UPrimitiveComponent : public USceneComponent
{

    UCLASS(UPrimitiveComponent, "PrimitiveComponent", USceneComponent)

public:

    virtual void Initialize() override;

    virtual FPrimitiveRenderData CreateRenderData(bool bSelected = false) const;

    // 로컬 공간 AABB. 피킹에 쓴다. 없으면 false.
    // 광선 검사(FRay)는 Editor 쪽 타입이라, 컴포넌트는 Bounds 데이터만 내준다 (Core <- Engine <- Editor)
    virtual bool GetLocalBounds(FVector& OutMin, FVector& OutMax) const;

    FMeshResource* GetMeshResource() const
    {
        return GResourceManager::GetInstance()->GetPrimitive(FString(GetInstanceClass()->Name));
    }
};

