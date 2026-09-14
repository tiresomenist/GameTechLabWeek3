#pragma once

#include "Core/Container/FString.h"
#include "Engine/Component/USceneComponent.h"
#include "Engine/Renderer/FPrimitiveRenderData.h"
#include "Engine/Resource/GResourceManager.h"
struct FMeshResource;
class UCameraComponent;

class UPrimitiveComponent : public USceneComponent
{

    UCLASS(UPrimitiveComponent, "PrimitiveComponent", USceneComponent)

public:

    virtual void Initialize() override;

    virtual FPrimitiveRenderData CreateRenderData(bool bSelected = false) const;

    // 로컬 공간 AABB. 피킹에 쓴다. 없으면 false.
    // 광선 검사(FRay)는 Editor 쪽 타입이라, 컴포넌트는 Bounds 데이터만 내준다 (Core <- Engine <- Editor)
    virtual bool GetLocalBounds(FVector& OutMin, FVector& OutMax) const;

    // 메시 삼각형 대신 AABB 자체를 선택 영역으로 쓰는 컴포넌트용 훅.
    virtual bool IsAABBOnlyPickable() const { return false; }

    // 렌더와 피킹에 쓰는 월드 행렬. 기본은 GetWorldMatrix()와 같다.
    // 카메라에 따라 모양이 바뀌는 컴포넌트(빌보드 텍스트 등)가 오버라이드한다.
    // 렌더(RenderUtil)와 피킹(FObjectPicker)이 같은 함수를 써야 보이는 곳과 클릭되는 곳이 일치한다.
    virtual const FMatrix& GetRenderWorldMatrix(const UCameraComponent* Camera) const;

    FMeshResource* GetMeshResource() const
    {
		return GResourceManager::GetInstance()->GetPrimitive(GetInstanceClass()->Name.ToString());
    }
};

