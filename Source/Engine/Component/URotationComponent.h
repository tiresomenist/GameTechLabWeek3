#pragma once

#include "Core/Math/FVector.h"
#include "Engine/Component/UActorComponent.h"

class USceneComponent;

// Pivot과 Owner가 같은 좌표계에 있다고 가정.
class URotationComponent : public UActorComponent
{
	UCLASS(URotationComponent, "RotationComponent", UActorComponent)
public:
	virtual void Tick(float DeltaTime) override;
	void SetRotation(float Speed, FVector Axis);
	void SetOrbit(float Speed, float Radius, FVector Axis);
	void SetPivot(USceneComponent* Comp);
	// 공전 중 로컬 +Z가 진행 방향을 향하게 함 (로켓처럼 긴 메시용)
	void SetFaceOrbitDirection(bool bFace);

private:
	USceneComponent* ResolvePivotTransform() const;

	AActor* PivotActor = nullptr;
	uint32 PivotComponentUUID = static_cast<uint32>(-1);
	float RotationSpeed = 0.0f;
	float OrbitSpeed = 0.0f;
	float OrbitRadius = 0.0f;
	float ElapsedTime = 0.0f;
	bool bFaceOrbitDirection = false;

	FVector RotationAxis = FVector(0.0f, 0.0f, 1.0f);
	FVector OrbitAxis = FVector(0.0f, 0.0f, 1.0f);
	FVector OrbitPlane = FVector(1.0f, 0.0f, 0.0f);

	FVector GetOrbitPlane(FVector Axis) const;
};
