#include "pch.h"
#include "URotationComponent.h"
#include "Core/Math/FQuaternion.h"
#include "Engine/Actor/AActor.h"
#include "Engine/Component/USceneComponent.h"

void URotationComponent::Tick(float DeltaTime)
{
	ElapsedTime += DeltaTime;

	if (!OwnerTransform)
	{
		AActor* Owner = GetOwner();
		if (!Owner)
		{
			return;
		}

		OwnerTransform = Owner->GetRootComponent();
		if (!OwnerTransform)
		{
			return;
		}
	}

	OwnerTransform->SetRelativeRotation(FQuaternion::FromAxisAngle(RotationAxis, RotationSpeed * ElapsedTime));

	if (!PivotTransform)
	{
		return;
	}

	FVector Offset = OrbitPlane * OrbitRadius;
	FVector Rotated = FQuaternion::FromAxisAngle(OrbitAxis, OrbitSpeed * ElapsedTime).ToRotationMatrix().TransformPosition(Offset);
	OwnerTransform->SetRelativeLocation(Rotated + PivotTransform->GetRelativeLocation());
	OwnerTransform->AddWorldRotation(FQuaternion::FromAxisAngle(OrbitAxis, OrbitSpeed * ElapsedTime));
}

void URotationComponent::SetRotation(float Speed, FVector Axis)
{
	RotationSpeed = Speed;
	RotationAxis = Axis;
	RotationAxis.Normalize();
}

void URotationComponent::SetOrbit(float Speed, float Radius, FVector Axis)
{
	OrbitSpeed = Speed;
	OrbitRadius = Radius;
	OrbitAxis = Axis;
	OrbitAxis.Normalize();
	OrbitPlane = GetOrbitPlane(OrbitAxis);
}

void URotationComponent::SetPivot(USceneComponent* Transform)
{
	PivotTransform = Transform;
}

FVector URotationComponent::GetOrbitPlane(FVector Axis) const
{
	FVector X(1.0f, 0.0f, 0.0f);
	if ((Axis - X).LengthSquared() <= 1e-5f)
	{
		X = FVector(0.0f, 1.0f, 0.0f);
	}

	FVector Ret = Axis.Cross(X);
	Ret.Normalize();

	return Ret;
}
