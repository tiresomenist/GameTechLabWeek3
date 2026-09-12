#pragma once

#include "Engine/Component/UActorComponent.h"
#include "Core/Core.h"
#include "Core/Math/FVector.h"
#include "Core/Math/FQuaternion.h"
#include "Engine/Object/FClassType.h"
#include "Core/Math/Matrix.h"

class USceneComponent : public UActorComponent
{

    UCLASS(USceneComponent, "SceneComponent", UActorComponent)

public:
	virtual bool CanBeRootComponent() const { return true; }

    FVector& GetRelativeLocation() { return RelativeLocation; };
	const FVector& GetRelativeLocation() const { return RelativeLocation; }
    const FQuaternion& GetRelativeRotation() const { return RelativeRotation; }
    FVector& GetRelativeScale3D() { return RelativeScale3D; };
	const FVector& GetRelativeScale3D() const { return RelativeScale3D; }

    void SetRelativeLocation(const FVector& Location);
    void SetRelativeRotation(const FQuaternion& Rotation);
    void AddLocalRotation(const FQuaternion& Delta);
    void AddWorldRotation(const FQuaternion& Delta);
    void SetRelativeScale3D(const FVector& Scale3D);

    const FMatrix& GetWorldMatrix() const;
    FVector GetWorldLocation() const
    {
        const FMatrix& WorldMat = GetWorldMatrix();
        return GetWorldMatrix().GetOrigin();
    }

    virtual void Serialize(FArchive& Archive) override;
    virtual void Deserialize(FArchive& Archive) override;

protected:
    // 로컬 트랜스폼
    FVector RelativeLocation;
    FQuaternion RelativeRotation;
    // Transform의 기본 스케일은 단위 스케일이어야 한다. FVector의 기본값은
    // (0, 0, 0)이므로 명시하지 않으면 메시 정점이 원점으로 붕괴한다.
    FVector RelativeScale3D{ 1.0f, 1.0f, 1.0f };

    // 계층 구조(구현X)
    USceneComponent* AttachParent = nullptr;
    std::vector<USceneComponent*> AttachChildren;

    // 최종 월드 행렬 캐싱 & 더티 플래그(구현X)
    mutable FMatrix CachedWorldMatrix;
    mutable bool bWorldMatrixDirty = true;

    void UpdateWorldTransform() const;
};

