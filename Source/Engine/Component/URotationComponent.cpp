#include "pch.h"
#include "URotationComponent.h"
#include "Core/Math/FQuaternion.h"
#include "Engine/Actor/AActor.h"
#include "Engine/Component/USceneComponent.h"

void URotationComponent::Tick(float DeltaTime)
{
	ElapsedTime += DeltaTime;

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}
	USceneComponent* OwnerTransform = Owner->GetRootComponent();
	if (!OwnerTransform) return;

	const FQuaternion SpinRotation = FQuaternion::FromAxisAngle(RotationAxis, RotationSpeed * ElapsedTime);
	OwnerTransform->SetRelativeRotation(SpinRotation);

	USceneComponent* PivotTransform = ResolvePivotTransform();
	if (!PivotTransform)
	{
		return;
	}

	FVector Offset = OrbitPlane * OrbitRadius;
	FVector Rotated = FQuaternion::FromAxisAngle(OrbitAxis, OrbitSpeed * ElapsedTime).ToRotationMatrix().TransformPosition(Offset);
	OwnerTransform->SetRelativeLocation(Rotated + PivotTransform->GetRelativeLocation());

	if (bFaceOrbitDirection)
	{
		// 잠시 뒤 위치와의 차이로 공전 진행 방향을 구하고 로컬 +Z(머리)를 그 방향으로 맞춤
		const float LookAheadTime = 0.01f;
		FVector Next = FQuaternion::FromAxisAngle(OrbitAxis, OrbitSpeed * (ElapsedTime + LookAheadTime)).ToRotationMatrix().TransformPosition(Offset);
		const FQuaternion FaceRotation = FQuaternion::FromToRotation(FVector(0.0f, 0.0f, 1.0f), Next - Rotated);
		// 자전을 먼저 적용해 머리 방향 축으로 롤 회전하게 함
		OwnerTransform->SetRelativeRotation(FaceRotation * SpinRotation);
		return;
	}

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

void URotationComponent::SetFaceOrbitDirection(bool bFace)
{
	bFaceOrbitDirection = bFace;
}

void URotationComponent::SetPivot(USceneComponent* Transform)
{
	PivotActor = Transform ? Transform->GetOwner() : nullptr;
	PivotComponentUUID = Transform ? Transform->GetUUID() : static_cast<uint32>(-1);
}

USceneComponent* URotationComponent::ResolvePivotTransform() const
{
	if (!PivotActor || PivotComponentUUID == static_cast<uint32>(-1)) return nullptr;

	for (UActorComponent* Component : PivotActor->GetComponents())
	{
		if (Component->GetUUID() == PivotComponentUUID && Component->IsA(USceneComponent::GetClass()))
		{
			return static_cast<USceneComponent*>(Component);
		}
	}
	return nullptr;
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
