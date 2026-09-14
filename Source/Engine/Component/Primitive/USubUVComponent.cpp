#include "pch.h"
#include "USubUVComponent.h"

#include "Engine/Component/UCameraComponent.h"
#include "Engine/Resource/GResourceManager.h"

void USubUVComponent::Initialize()
{
	Super::Initialize();
	SetMaterial(GResourceManager::GetInstance()->LoadTexture("Assets/Textures/FlameTexture.png"));
	SetBlendMode(EPrimitiveBlendMode::Additive);
}

void USubUVComponent::Tick(float DeltaTime)
{
	AnimationTime += DeltaTime;
	FrameIndex = static_cast<uint32>(AnimationTime * FramesPerSecond) % (Columns * Rows);
}

FPrimitiveRenderData USubUVComponent::CreateRenderData(bool bSelected) const
{
	FPrimitiveRenderData OutData = Super::CreateRenderData(bSelected);

	const uint32 Column = FrameIndex % Columns;
	const uint32 Row = FrameIndex / Columns;
	OutData.SubUV = FVector4(
		static_cast<float>(Column) / Columns,
		static_cast<float>(Row) / Rows,
		1.0f / Columns,
		1.0f / Rows);

	return OutData;
}

FMeshResource* USubUVComponent::GetMeshResource() const
{
	return GResourceManager::GetInstance()->GetPrimitive("Plane");
}

const FMatrix& USubUVComponent::GetRenderWorldMatrix(const UCameraComponent* Camera) const
{
	if (!Camera) return GetWorldMatrix();

	// Plane은 XY 평면(법선 +Z), 텍스트 빌보드는 YZ 평면(법선 -X)을 쓴다.
	// Plane의 앞면이 카메라를 향하도록 가로 X축을 빌보드 가로 Y축으로,
	// 세로 Y축을 빌보드 세로 -Z축으로 맞춘다.
	const FMatrix PlaneToBillboard =
		FMatrix::MakeRotationYMatrix(-PI / 2.0f) *
		FMatrix::MakeRotationXMatrix(-PI / 2.0f);

	BillboardWorldMatrix = FMatrix::MakeScaleMatrix(GetRelativeScale3D())
		* PlaneToBillboard
		* Camera->GetRelativeRotation().ToRotationMatrix()
		* FMatrix::MakeTranslationMatrix(GetRelativeLocation());
	return BillboardWorldMatrix;
}
