#pragma once

#include "Engine/Scene/UScene.h"
#include "Engine/Component/URotationComponent.h"
#include "Engine/Component/UStaticMeshComponent.h"
#include "Engine/Component/UWidgetComponent.h"

inline void SpawnSolarSystem(UScene* Scene)
{
	// 태양
	AActor* Sun = Scene->SpawnActor<AActor*>(AActor::GetClass());
	Sun->CreateComponent(UWidgetComponent::GetClass());

	auto* SunMesh = static_cast<UStaticMeshComponent*>(Sun->CreateComponent(UStaticMeshComponent::GetClass()));
	SunMesh->SetStaticMesh("TexturedSphere");
	SunMesh->SetMaterial("Assets/Textures/sun.png");

	auto* SunRot = static_cast<URotationComponent*>(Sun->CreateComponent(URotationComponent::GetClass()));
	SunRot->SetRotation(1.0f, FVector(0.0f, 0.0f, 1.0f));

	// 지구
	AActor* Earth = Scene->SpawnActor<AActor*>(AActor::GetClass());
	Earth->CreateComponent(UWidgetComponent::GetClass());

	auto* EarthMesh = static_cast<UStaticMeshComponent*>(Earth->CreateComponent(UStaticMeshComponent::GetClass()));
	EarthMesh->SetStaticMesh("TexturedSphere");
	EarthMesh->SetMaterial("Assets/Textures/earth.png");

	auto* EarthRot = static_cast<URotationComponent*>(Earth->CreateComponent(URotationComponent::GetClass()));
	EarthRot->SetRotation(1.0f, FVector(0.0f, 0.0f, 1.0f));
	EarthRot->SetOrbit(1.0f, 3.0f, FVector(0.0f, 1.0f, 1.0f));
	EarthRot->SetPivot(SunMesh);
}