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

		for (const auto Comp : Owner->GetComponents())
		{
			if (Comp->IsA(USceneComponent::GetClass()))
			{
				OwnerTransform = static_cast<USceneComponent*>(Comp);
				break;
			}
		}

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

	OwnerTransform->SetRelativeLocation(OrbitAxis * OrbitRadius);
	OwnerTransform->AddLocalRotation(FQuaternion::FromAxisAngle(OrbitAxis, OrbitSpeed * ElapsedTime));
	OwnerTransform->SetRelativeLocation(OwnerTransform->GetRelativeLocation() + PivotTransform->GetRelativeLocation());
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
}

void URotationComponent::SetPivot(USceneComponent* Transform)
{
	PivotTransform = Transform;
}
