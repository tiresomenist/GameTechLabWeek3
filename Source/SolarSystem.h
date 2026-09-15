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
	SunMesh->SetStaticMesh("Sphere");
	SunMesh->SetMaterial("Assets/Textures/sun.png");
	SunMesh->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));

	auto* SunRot = static_cast<URotationComponent*>(Sun->CreateComponent(URotationComponent::GetClass()));
	SunRot->SetRotation(1.0f, FVector(0.0f, 0.0f, 1.0f));

	// 수성
	AActor* Mercury = Scene->SpawnActor<AActor*>(AActor::GetClass());
	Mercury->CreateComponent(UWidgetComponent::GetClass());

	auto* MercuryMesh = static_cast<UStaticMeshComponent*>(Mercury->CreateComponent(UStaticMeshComponent::GetClass()));
	MercuryMesh->SetStaticMesh("Sphere");
	MercuryMesh->SetMaterial("Assets/Textures/mercury.png");
	MercuryMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));

	auto* MercuryRot = static_cast<URotationComponent*>(Mercury->CreateComponent(URotationComponent::GetClass()));
	MercuryRot->SetRotation(1.0f, FVector(0.0f, 0.0f, 1.0f));
	MercuryRot->SetOrbit(1.0f, 5.0f, FVector(0.0f, 0.0f, 1.0f));
	MercuryRot->SetPivot(SunMesh);

	// 금성
	AActor* Venus = Scene->SpawnActor<AActor*>(AActor::GetClass());
	Venus->CreateComponent(UWidgetComponent::GetClass());

	auto* VenusMesh = static_cast<UStaticMeshComponent*>(Venus->CreateComponent(UStaticMeshComponent::GetClass()));
	VenusMesh->SetStaticMesh("Sphere");
	VenusMesh->SetMaterial("Assets/Textures/venus.png");
	VenusMesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.6f));

	auto* VenusRot = static_cast<URotationComponent*>(Venus->CreateComponent(URotationComponent::GetClass()));
	VenusRot->SetRotation(1.0f, FVector(0.0f, 0.0f, 1.0f));
	VenusRot->SetOrbit(0.8f, 8.0f, FVector(0.0f, 0.0f, 1.0f));
	VenusRot->SetPivot(SunMesh);

	// 지구
	AActor* Earth = Scene->SpawnActor<AActor*>(AActor::GetClass());
	Earth->CreateComponent(UWidgetComponent::GetClass());

	auto* EarthMesh = static_cast<UStaticMeshComponent*>(Earth->CreateComponent(UStaticMeshComponent::GetClass()));
	EarthMesh->SetStaticMesh("Sphere");
	EarthMesh->SetMaterial("Assets/Textures/earth.png");
	EarthMesh->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.7f));

	auto* EarthRot = static_cast<URotationComponent*>(Earth->CreateComponent(URotationComponent::GetClass()));
	EarthRot->SetRotation(1.0f, FVector(0.0f, 0.0f, 1.0f));
	EarthRot->SetOrbit(0.6f, 11.0f, FVector(0.0f, 0.0f, 1.0f));
	EarthRot->SetPivot(SunMesh);

	// 달
	AActor* Moon = Scene->SpawnActor<AActor*>(AActor::GetClass());
	Moon->CreateComponent(UWidgetComponent::GetClass());

	auto* MoonMesh = static_cast<UStaticMeshComponent*>(Moon->CreateComponent(UStaticMeshComponent::GetClass()));
	MoonMesh->SetStaticMesh("Sphere");
	MoonMesh->SetMaterial("Assets/Textures/moon.png");
	MoonMesh->SetRelativeScale3D(FVector(0.2f, 0.2f, 0.2f));

	auto* MoonRot = static_cast<URotationComponent*>(Moon->CreateComponent(URotationComponent::GetClass()));
	MoonRot->SetRotation(1.0f, FVector(0.0f, 0.0f, 1.0f));
	MoonRot->SetOrbit(-1.0f, 1.5f, FVector(0.0f, 0.0f, 1.0f));
	MoonRot->SetPivot(EarthMesh);
}