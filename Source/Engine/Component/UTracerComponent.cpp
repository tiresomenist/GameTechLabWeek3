#include "pch.h"
#include "UTracerComponent.h"
#include "Core/Math/FQuaternion.h"
#include "Engine/Actor/AActor.h"
#include "Engine/Component/USceneComponent.h"

void UTracerComponent::Tick(float DeltaTime)
{
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

	if (!TargetTransform)
	{
		return;
	}

	FVector OwnerLocation = OwnerTransform->GetRelativeLocation();
	FVector Direction = TargetTransform->GetRelativeLocation() - OwnerLocation;
	Direction.Normalize();
	OwnerTransform->SetRelativeLocation(OwnerLocation + Direction * TraceSpeed * DeltaTime);
	OwnerTransform->SetRelativeRotation(FQuaternion::FromToRotation(FVector(0.0f, 0.0f, 1.0f), Direction));
}

void UTracerComponent::SetTarget(USceneComponent* Target)
{
	TargetTransform = Target;
}

void UTracerComponent::SetSpeed(float Speed)
{
	TraceSpeed = Speed;
}
