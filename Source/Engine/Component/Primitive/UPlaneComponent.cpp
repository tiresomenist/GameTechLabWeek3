#include "pch.h"
#include "UPlaneComponent.h"
#include "Engine/Resource/GResourceManager.h"

void UPlaneComponent::Initialize()
{
	Super::Initialize();
	SetMaterial(GResourceManager::GetInstance()->LoadTexture("Assets/Textures/FlameTexture.png"));
}
