#include "pch.h"
#include "USceneComponent.h"
#include "Engine/Object/UObject.h"
#include "Engine/Object/FObjectFactory.h"
#include "Engine/Object/FArchive.h"
#include "Engine/Actor/AActor.h"
#include "Core/Math/FQuaternion.h"
namespace
{
    bool IsSameRotation(const FQuaternion& Left,const FQuaternion& Right)
    {
        FQuaternion A = Left;
        FQuaternion B = Right;

        A.Normalize();
        B.Normalize();

        const auto Square = [](double Value) {return Value * Value;};

        // 같은 부호로 표현된 Quaternion의 차이 계산함
        const double SameSignDistance =
            Square(static_cast<double>(A.X) - B.X) +
            Square(static_cast<double>(A.Y) - B.Y) +
            Square(static_cast<double>(A.Z) - B.Z) +
            Square(static_cast<double>(A.W) - B.W);

        // 반대 부호로 표현된 동일 자세까지 비교함
        const double OppositeSignDistance =
            Square(static_cast<double>(A.X) + B.X) +
            Square(static_cast<double>(A.Y) + B.Y) +
            Square(static_cast<double>(A.Z) + B.Z) +
            Square(static_cast<double>(A.W) + B.W);

        constexpr double ToleranceSquared = 1.0e-12;

        return (std::min)(SameSignDistance, OppositeSignDistance) <= ToleranceSquared;
    }
}

USceneComponent::~USceneComponent()
{
    DetachFromParent();

    // 부모가 먼저 제거되어도 자식이 해제된 부모를 참조하지 않게 한다.
    const std::vector<USceneComponent*> Children = AttachChildren;
    AttachChildren.clear();
    for (USceneComponent* Child : Children)
    {
        if (Child != nullptr && Child->AttachParent == this)
        {
            Child->AttachParent = nullptr;
            Child->UpdateWorldTransform();
        }
    }
}

bool USceneComponent::AttachTo(USceneComponent* Parent)
{
    if (Parent == nullptr || Parent == this || Parent == AttachParent)
    {
        return Parent == AttachParent;
    }

    // Component 계층은 Actor 경계를 넘지 않는다.
    if (GetOwner() == nullptr || GetOwner() != Parent->GetOwner())
    {
        return false;
    }

    // Parent의 상위 체인에 this가 있으면 순환 참조가 된다.
    for (USceneComponent* Ancestor = Parent; Ancestor != nullptr; Ancestor = Ancestor->AttachParent)
    {
        if (Ancestor == this)
        {
            return false;
        }
    }

    DetachFromParent();
    AttachParent = Parent;
    AttachParent->AttachChildren.push_back(this);
    UpdateWorldTransform();
    return true;
}

void USceneComponent::DetachFromParent()
{
    if (AttachParent == nullptr)
    {
        return;
    }

    std::vector<USceneComponent*>& Siblings = AttachParent->AttachChildren;
    Siblings.erase(std::remove(Siblings.begin(), Siblings.end(), this), Siblings.end());
    AttachParent = nullptr;
    UpdateWorldTransform();
}

void USceneComponent::SetRelativeLocation(const FVector& Location)
{
    if (!std::isfinite(Location.X) || !std::isfinite(Location.Y) || !std::isfinite(Location.Z)) return;
	RelativeLocation = Location;
    UpdateWorldTransform();
}

void USceneComponent::SetRelativeRotation(const FQuaternion& Rotation)
{
    FQuaternion NewRotation = Rotation;
    NewRotation.Normalize();
     if (IsSameRotation(RelativeRotation, NewRotation))
    {
        return;
    }
    const FRotator Extracted = FRotator::FromQuaternion(NewRotation);
    // 기존 표시값에 가까운 각도로 보정함
    RelativeRotator = Extracted.GetUnwoundNear(RelativeRotator);

    RelativeRotation = NewRotation;
    UpdateWorldTransform();
}

void USceneComponent::AddLocalRotation(const FQuaternion& Delta)
{
    SetRelativeRotation(RelativeRotation * Delta);
}

void USceneComponent::AddWorldRotation(const FQuaternion& Delta)
{
    // 현재 부모가 없어서 합성할 부모 회전각이 없음
    SetRelativeRotation(Delta * RelativeRotation);
}

void USceneComponent::SetRelativeScale3D(const FVector& Scale3D)
{
    if (!std::isfinite(Scale3D.X) || !std::isfinite(Scale3D.Y) || !std::isfinite(Scale3D.Z)) return;
	RelativeScale3D = Scale3D;
    UpdateWorldTransform();
}

const FMatrix& USceneComponent::GetWorldMatrix() const
{
    //if (bWorldMatrixDirty)
    return CachedWorldMatrix;
}

void USceneComponent::UpdateWorldTransform() const
{
    FMatrix LocalSRTMatrix = FMatrix::MakeScaleMatrix(RelativeScale3D)
        * RelativeRotation.ToRotationMatrix()
        * FMatrix::MakeTranslationMatrix(RelativeLocation);

    // 이 프로젝트는 row vector 규약이므로 Local * Parent 순서로 합성한다.
    CachedWorldMatrix = AttachParent
        ? LocalSRTMatrix * AttachParent->GetWorldMatrix()
        : LocalSRTMatrix;

    // 부모 Transform이 바뀌면 모든 자식의 캐시도 즉시 갱신한다.
    for (USceneComponent* Child : AttachChildren)
    {
        if (Child != nullptr)
        {
            Child->UpdateWorldTransform();
        }
    }
}

void USceneComponent::Serialize(FArchive& Archive)
{
    Super::Serialize(Archive);

    // Location
    TArray<float> Location
    {
        RelativeLocation.X,
        RelativeLocation.Y,
        RelativeLocation.Z,
    };
    Archive.SetArray<float>("Location", Location);

    // Rotation
    TArray<float> Rotation
    {
        RelativeRotator.Roll,
        RelativeRotator.Pitch,
        RelativeRotator.Yaw
    };

    Archive.SetArray<float>("Rotation", Rotation);

    // Scale
    TArray<float> Scale
    {
        RelativeScale3D.X,
        RelativeScale3D.Y,
        RelativeScale3D.Z,
    };
    Archive.SetArray<float>("Scale", Scale);
}

void USceneComponent::Deserialize(FArchive& Archive)
{
    Super::Deserialize(Archive);
    const auto Location = Archive.GetVector3OrDefault("Location", 0.0f);
    const auto Rotation = Archive.GetVector3OrDefault("Rotation", 0.0f);
    const auto Scale = Archive.GetVector3OrDefault("Scale", 1.0f);
    RelativeLocation = FVector(Location[0], Location[1], Location[2]);
    RelativeRotator = FRotator(Rotation[1], Rotation[2], Rotation[0]);
    RelativeRotation = RelativeRotator.ToQuaternion();
    RelativeScale3D = FVector(Scale[0], Scale[1], Scale[2]);
    UpdateWorldTransform();
}

void USceneComponent::SetRelativeRotation(const FRotator& Rotation)
{
    if (!Rotation.IsFinite()) return;
    RelativeRotator = Rotation;
    RelativeRotation = Rotation.ToQuaternion();
    UpdateWorldTransform();
}
