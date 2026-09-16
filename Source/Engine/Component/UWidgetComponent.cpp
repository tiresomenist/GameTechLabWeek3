#include "pch.h"
#include "UWidgetComponent.h"

#include "Engine/Actor/AActor.h"
#include "Engine/Component/UCameraComponent.h"
#include "Engine/Component/Primitive/UPrimitiveComponent.h"

bool UWidgetComponent::BuildTextItem(const UCameraComponent* Camera, FWorldTextItem& OutItem) const
{
	if (!Camera || !GetOwner()) return false;

	USceneComponent* Root = GetOwner()->GetRootComponent();
	if (!Root || !Root->IsA(UPrimitiveComponent::GetClass())) return false;

	auto* Primitive = static_cast<UPrimitiveComponent*>(Root);
	FVector BoundsMin;
	FVector BoundsMax;
	if (!Primitive->GetLocalBounds(BoundsMin, BoundsMax)) return false;

	// 로컬 상단점을 회전하면 라벨도 함께 내려가므로, 변환된 8개 코너로 월드 AABB를 만든다.
	const FMatrix& RenderWorld = Primitive->GetRenderWorldMatrix(Camera);
	FVector WorldMin;
	FVector WorldMax;
	for (uint32 Corner = 0; Corner < 8; ++Corner)
	{
		const FVector LocalCorner(
			(Corner & 1) ? BoundsMax.X : BoundsMin.X,
			(Corner & 2) ? BoundsMax.Y : BoundsMin.Y,
			(Corner & 4) ? BoundsMax.Z : BoundsMin.Z);
		const FVector WorldCorner = RenderWorld.TransformPosition(LocalCorner);
		if (Corner == 0)
		{
			WorldMin = WorldCorner;
			WorldMax = WorldCorner;
			continue;
		}
		WorldMin.X = (std::min)(WorldMin.X, WorldCorner.X);
		WorldMin.Y = (std::min)(WorldMin.Y, WorldCorner.Y);
		WorldMin.Z = (std::min)(WorldMin.Z, WorldCorner.Z);
		WorldMax.X = (std::max)(WorldMax.X, WorldCorner.X);
		WorldMax.Y = (std::max)(WorldMax.Y, WorldCorner.Y);
		WorldMax.Z = (std::max)(WorldMax.Z, WorldCorner.Z);
	}

	const FVector Anchor(
		(WorldMin.X + WorldMax.X) * 0.5f,
		(WorldMin.Y + WorldMax.Y) * 0.5f,
		WorldMax.Z);
	const FVector FinalAnchor = Anchor + GetRelativeLocation() + FVector::Up * 0.3f;
	OutItem.Text = std::to_string(Primitive->GetUUID());
	OutItem.WorldMatrix = FMatrix::MakeScaleMatrix(GetRelativeScale3D())
		* Camera->GetRelativeRotation().ToRotationMatrix()
		* FMatrix::MakeTranslationMatrix(FinalAnchor);
	return true;
}
