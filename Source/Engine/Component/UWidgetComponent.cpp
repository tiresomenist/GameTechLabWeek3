#include "pch.h"
#include "UWidgetComponent.h"

#include "Engine/Actor/UActor.h"
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

	// Widget의 위치는 Root 로컬 공간의 추가 오프셋이며, 라벨은 메시 상단보다 0.3 위에 둔다.
	const FVector LocalAnchor = FVector(0.0f, 0.0f, BoundsMax.Z) + GetRelativeLocation();
	const FVector Anchor = Primitive->GetWorldMatrix().TransformPosition(LocalAnchor) + FVector::Up * 0.3f;
	OutItem.Text = std::to_string(Primitive->GetUUID());
	OutItem.WorldMatrix = FMatrix::MakeScaleMatrix(GetRelativeScale3D())
		* Camera->GetRelativeRotation().ToRotationMatrix()
		* FMatrix::MakeTranslationMatrix(Anchor);
	return true;
}
