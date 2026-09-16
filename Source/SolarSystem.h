#pragma once

#include "Engine/Scene/UScene.h"
#include "Engine/Component/URotationComponent.h"
#include "Engine/Component/UStaticMeshComponent.h"
#include "Engine/Component/UWidgetComponent.h"
#include "Engine/Resource/FMeshNames.h"

inline void SpawnSolarSystem(UScene* Scene)
{
	// 태양
	AActor* Sun = Scene->SpawnActor<AActor*>(AActor::GetClass());
	Sun->CreateComponent(UWidgetComponent::GetClass());

	auto* SunMesh = static_cast<UStaticMeshComponent*>(Sun->CreateComponent(UStaticMeshComponent::GetClass()));
	SunMesh->SetStaticMesh(GetMeshNames().Sphere);
	SunMesh->SetMaterial("Assets/Textures/sun.png");
	SunMesh->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));

	auto* SunRot = static_cast<URotationComponent*>(Sun->CreateComponent(URotationComponent::GetClass()));
	SunRot->SetRotation(-0.1f, FVector(0.0f, 0.0f, 1.0f));

	// 수성
	AActor* Mercury = Scene->SpawnActor<AActor*>(AActor::GetClass());
	Mercury->SetParentActor(Sun);
	Mercury->CreateComponent(UWidgetComponent::GetClass());

	auto* MercuryMesh = static_cast<UStaticMeshComponent*>(Mercury->CreateComponent(UStaticMeshComponent::GetClass()));
	MercuryMesh->SetStaticMesh(GetMeshNames().Sphere);
	MercuryMesh->SetMaterial("Assets/Textures/mercury.png");
	MercuryMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));

	auto* MercuryRot = static_cast<URotationComponent*>(Mercury->CreateComponent(URotationComponent::GetClass()));
	MercuryRot->SetRotation(-1.0f, FVector(0.0f, 0.0f, 1.0f));
	MercuryRot->SetOrbit(-1.0f, 5.0f, FVector(0.0f, 0.0f, 1.0f));
	MercuryRot->SetPivot(SunMesh);

	// 금성
	AActor* Venus = Scene->SpawnActor<AActor*>(AActor::GetClass());
	Venus->SetParentActor(Sun);
	Venus->CreateComponent(UWidgetComponent::GetClass());

	auto* VenusMesh = static_cast<UStaticMeshComponent*>(Venus->CreateComponent(UStaticMeshComponent::GetClass()));
	VenusMesh->SetStaticMesh(GetMeshNames().Sphere);
	VenusMesh->SetMaterial("Assets/Textures/venus.png");
	VenusMesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.6f));

	auto* VenusRot = static_cast<URotationComponent*>(Venus->CreateComponent(URotationComponent::GetClass()));
	VenusRot->SetRotation(0.8f, FVector(0.0f, 0.0f, 1.0f));
	VenusRot->SetOrbit(-0.8f, 8.0f, FVector(0.0f, 0.0f, 1.0f));
	VenusRot->SetPivot(SunMesh);

	// 지구
	AActor* Earth = Scene->SpawnActor<AActor*>(AActor::GetClass());
	Earth->SetParentActor(Sun);
	Earth->CreateComponent(UWidgetComponent::GetClass());

	auto* EarthMesh = static_cast<UStaticMeshComponent*>(Earth->CreateComponent(UStaticMeshComponent::GetClass()));
	EarthMesh->SetStaticMesh(GetMeshNames().Sphere);
	EarthMesh->SetMaterial("Assets/Textures/earth.png");
	EarthMesh->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.7f));

	auto* EarthRot = static_cast<URotationComponent*>(Earth->CreateComponent(URotationComponent::GetClass()));
	EarthRot->SetRotation(-1.0f, FVector(0.0f, 0.3f, 1.0f));
	EarthRot->SetOrbit(-0.6f, 11.0f, FVector(0.0f, 0.0f, 1.0f));
	EarthRot->SetPivot(SunMesh);

	// 달
	AActor* Moon = Scene->SpawnActor<AActor*>(AActor::GetClass());
	Moon->SetParentActor(Earth);
	Moon->CreateComponent(UWidgetComponent::GetClass());

	auto* MoonMesh = static_cast<UStaticMeshComponent*>(Moon->CreateComponent(UStaticMeshComponent::GetClass()));
	MoonMesh->SetStaticMesh(GetMeshNames().Sphere);
	MoonMesh->SetMaterial("Assets/Textures/moon.png");
	MoonMesh->SetRelativeScale3D(FVector(0.2f, 0.2f, 0.2f));

	auto* MoonRot = static_cast<URotationComponent*>(Moon->CreateComponent(URotationComponent::GetClass()));
	// MoonRot->SetRotation(1.0f, FVector(0.0f, 0.0f, 1.0f));
	MoonRot->SetOrbit(-2.0f, 1.5f, FVector(0.0f, 0.3f, 1.0f));
	MoonRot->SetPivot(EarthMesh);

	// 화성
	AActor* Mars = Scene->SpawnActor<AActor*>(AActor::GetClass());
	Mars->SetParentActor(Sun);
	Mars->CreateComponent(UWidgetComponent::GetClass());

	auto* MarsMesh = static_cast<UStaticMeshComponent*>(Mars->CreateComponent(UStaticMeshComponent::GetClass()));
	MarsMesh->SetStaticMesh(GetMeshNames().Sphere);
	MarsMesh->SetMaterial("Assets/Textures/mars.png");
	MarsMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.8f));

	auto* MarsRot = static_cast<URotationComponent*>(Mars->CreateComponent(URotationComponent::GetClass()));
	MarsRot->SetRotation(-1.0f, FVector(0.0f, 0.3f, 1.0f));
	MarsRot->SetOrbit(-0.5f, 15.0f, FVector(0.0f, 0.0f, 1.0f));
	MarsRot->SetPivot(SunMesh);

	// 목성
	AActor* Jupiter = Scene->SpawnActor<AActor*>(AActor::GetClass());
	Jupiter->SetParentActor(Sun);
	Jupiter->CreateComponent(UWidgetComponent::GetClass());

	auto* JupiterMesh = static_cast<UStaticMeshComponent*>(Jupiter->CreateComponent(UStaticMeshComponent::GetClass()));
	JupiterMesh->SetStaticMesh(GetMeshNames().Sphere);
	JupiterMesh->SetMaterial("Assets/Textures/jupiter.png");
	JupiterMesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 1.5f));

	auto* JupiterRot = static_cast<URotationComponent*>(Jupiter->CreateComponent(URotationComponent::GetClass()));
	JupiterRot->SetRotation(-3.0f, FVector(0.0f, 0.0f, 1.0f));
	JupiterRot->SetOrbit(-0.1f, 20.0f, FVector(0.0f, 0.0f, 1.0f));
	JupiterRot->SetPivot(SunMesh);

	// 목성 위성 1
	AActor* Makemake = Scene->SpawnActor<AActor*>(AActor::GetClass());
	Makemake->SetParentActor(Jupiter);
	Makemake->CreateComponent(UWidgetComponent::GetClass());

	auto* MakemakeMesh = static_cast<UStaticMeshComponent*>(Makemake->CreateComponent(UStaticMeshComponent::GetClass()));
	MakemakeMesh->SetStaticMesh(GetMeshNames().Sphere);
	MakemakeMesh->SetMaterial("Assets/Textures/makemake.png");
	MakemakeMesh->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));

	auto* MakemakeRot = static_cast<URotationComponent*>(Makemake->CreateComponent(URotationComponent::GetClass()));
	// MoonRot->SetRotation(1.0f, FVector(0.0f, 0.0f, 1.0f));
	MakemakeRot->SetOrbit(-1.0f, 2.5f, FVector(0.0f, 1.0f, 1.0f));
	MakemakeRot->SetPivot(JupiterMesh);

	// 목성 위성 2
	AActor* Ceres = Scene->SpawnActor<AActor*>(AActor::GetClass());
	Ceres->SetParentActor(Jupiter);
	Ceres->CreateComponent(UWidgetComponent::GetClass());

	auto* CeresMesh = static_cast<UStaticMeshComponent*>(Ceres->CreateComponent(UStaticMeshComponent::GetClass()));
	CeresMesh->SetStaticMesh(GetMeshNames().Sphere);
	CeresMesh->SetMaterial("Assets/Textures/ceres.png");
	CeresMesh->SetRelativeScale3D(FVector(0.4f, 0.4f, 0.4f));

	auto* CeresRot = static_cast<URotationComponent*>(Ceres->CreateComponent(URotationComponent::GetClass()));
	// MoonRot->SetRotation(1.0f, FVector(0.0f, 0.0f, 1.0f));
	CeresRot->SetOrbit(-0.9f, 3.0f, FVector(1.0f, 0.0f, 1.0f));
	CeresRot->SetPivot(JupiterMesh);

	// 토성
	AActor* Saturn = Scene->SpawnActor<AActor*>(AActor::GetClass());
	Saturn->SetParentActor(Sun);
	Jupiter->CreateComponent(UWidgetComponent::GetClass());

	auto* SaturnMesh = static_cast<UStaticMeshComponent*>(Saturn->CreateComponent(UStaticMeshComponent::GetClass()));
	SaturnMesh->SetStaticMesh(GetMeshNames().Sphere);
	SaturnMesh->SetMaterial("Assets/Textures/saturn.png");
	SaturnMesh->SetRelativeScale3D(FVector(1.3f, 1.3f, 1.3f));

	auto* SaturnRot = static_cast<URotationComponent*>(Saturn->CreateComponent(URotationComponent::GetClass()));
	SaturnRot->SetRotation(-1.0f, FVector(0.0f, 0.0f, 1.0f));
	SaturnRot->SetOrbit(-0.2f, 25.0f, FVector(0.0f, 0.0f, 1.0f));
	SaturnRot->SetPivot(SunMesh);

	// 천왕성
	AActor* Uranus = Scene->SpawnActor<AActor*>(AActor::GetClass());
	Uranus->SetParentActor(Sun);
	Uranus->CreateComponent(UWidgetComponent::GetClass());

	auto* UranusMesh = static_cast<UStaticMeshComponent*>(Uranus->CreateComponent(UStaticMeshComponent::GetClass()));
	UranusMesh->SetStaticMesh(GetMeshNames().Sphere);
	UranusMesh->SetMaterial("Assets/Textures/uranus.png");
	UranusMesh->SetRelativeScale3D(FVector(1.3f, 1.3f, 1.3f));

	auto* UranusRot = static_cast<URotationComponent*>(Uranus->CreateComponent(URotationComponent::GetClass()));
	UranusRot->SetRotation(1.0f, FVector(0.0f, 1.0f, 0.0f));
	UranusRot->SetOrbit(-0.1f, 30.0f, FVector(0.0f, 0.0f, 1.0f));
	UranusRot->SetPivot(SunMesh);

	// 해왕성
	AActor* Neptune = Scene->SpawnActor<AActor*>(AActor::GetClass());
	Neptune->SetParentActor(Sun);
	Uranus->CreateComponent(UWidgetComponent::GetClass());

	auto* NeptuneMesh = static_cast<UStaticMeshComponent*>(Neptune->CreateComponent(UStaticMeshComponent::GetClass()));
	NeptuneMesh->SetStaticMesh(GetMeshNames().Sphere);
	NeptuneMesh->SetMaterial("Assets/Textures/neptune.png");
	NeptuneMesh->SetRelativeScale3D(FVector(1.3f, 1.3f, 1.3f));

	auto* NeptuneRot = static_cast<URotationComponent*>(Neptune->CreateComponent(URotationComponent::GetClass()));
	NeptuneRot->SetRotation(-1.0f, FVector(0.0f, 0.3f, 1.0f));
	NeptuneRot->SetOrbit(-0.05f, 35.0f, FVector(0.0f, 0.0f, 1.0f));
	NeptuneRot->SetPivot(SunMesh);

	// 배경
	AActor* Stars = Scene->SpawnActor<AActor*>(AActor::GetClass());
	Stars->SetParentActor(Sun);

	auto* StarsMesh = static_cast<UStaticMeshComponent*>(Stars->CreateComponent(UStaticMeshComponent::GetClass()));
	StarsMesh->SetStaticMesh(GetMeshNames().Sphere);
	StarsMesh->SetMaterial("Assets/Textures/stars.png");
	StarsMesh->SetRelativeScale3D(FVector(-100.0f, -100.0f, -100.0f));

	auto* StarsRot = static_cast<URotationComponent*>(Stars->CreateComponent(URotationComponent::GetClass()));
	StarsRot->SetRotation(0.05f, FVector(1.0f, 1.0f, 1.0f));
}
