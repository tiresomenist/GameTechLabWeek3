#include "pch.h"
#include "USceneComponent.h"
#include "Engine/Object/UObject.h"
#include "Engine/Object/FObjectFactory.h"
#include "Engine/Object/FArchive.h"
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

    CachedWorldMatrix = LocalSRTMatrix;

    // 부모가 있으면 부모의 월드 행렬과 곱함
    //if (AttachParent)
    //{
    //    CachedWorldMatrix = LocalSRTMatrix * AttachParent->GetWorldMatrix();
    //}
    //else
    //{
    //    CachedWorldMatrix = LocalSRTMatrix;
    //}

    //bWorldMatrixDirty = false;
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
