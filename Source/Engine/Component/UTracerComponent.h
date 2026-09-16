#pragma once

#include "Core/Math/FVector.h"
#include "Engine/Component/UActorComponent.h"

class USceneComponent;

class UTracerComponent : public UActorComponent
{
	UCLASS(UTracerComponent, "TracerComponent", UActorComponent)
public:
	virtual void Tick(float DeltaTime) override;
	void SetTarget(USceneComponent* Target);
	void SetSpeed(float Speed);

private:
	USceneComponent* OwnerTransform = nullptr;
	USceneComponent* TargetTransform = nullptr;

	float TraceSpeed = 1.0f;
};

