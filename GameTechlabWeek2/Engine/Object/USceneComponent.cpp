#include "pch.h"
#include "USceneComponent.h"
#include "Engine/Object/UObject.h"
#include "Engine/Object/FObjectFactory.h"
#include "Engine/Object/FArchive.h"
#include "FQuaternion.h"

void USceneComponent::SetRelativeLocation(const FVector& Location)
{
	RelativeLocation = Location;
    UpdateWorldTransform();
}

void USceneComponent::SetRelativeRotation(const FQuaternion& Rotation)
{
	RelativeRotation = Rotation;
    RelativeRotation.Normalize();
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
    FVector EulerRotation = FQuaternion::ToEuler(RelativeRotation);
    TArray<float> Rotation
    {
        EulerRotation.X,
        EulerRotation.Y,
        EulerRotation.Z,
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

    // Location
    TArray<float> Location = Archive.GetArray<float>("Location");
    if (Location.Num() != 3) return;
    RelativeLocation.X = Location[0];
    RelativeLocation.Y = Location[1];
    RelativeLocation.Z = Location[2];

    // Rotation
    TArray<float> Rotation = Archive.GetArray<float>("Rotation");
    if (Rotation.Num() != 3) return;

    FVector EulerRotation
    {
        Rotation[0],
        Rotation[1],
        Rotation[2],
    };
    RelativeRotation = FQuaternion::FromEuler(EulerRotation);
    
    // Scale
    TArray<float> Scale = Archive.GetArray<float>("Scale");
    if (Scale.Num() != 3) return;

    RelativeScale3D.X = Scale[0];
    RelativeScale3D.Y = Scale[1];
    RelativeScale3D.Z = Scale[2];

    UpdateWorldTransform();
}
