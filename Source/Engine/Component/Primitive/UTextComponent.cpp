#include "pch.h"
#include "UTextComponent.h"

#include "Engine/Component/UCameraComponent.h"
#include "Engine/Object/FArchive.h"
#include "Engine/Renderer/Text/FTextMeshBuilder.h"
#include "Engine/Resource/GResourceManager.h"

bool UTextComponent::GetLocalBounds(FVector& OutMin, FVector& OutMax) const
{
	FFontAtlas* Font = GResourceManager::GetInstance()->GetDefaultFont();
	if (!Font || !FTextMeshBuilder::GetLocalBounds(Text, *Font, OutMin, OutMax)) return false;

	OutMin -= FVector(SelectionPadding, SelectionPadding, SelectionPadding);
	OutMax += FVector(SelectionPadding, SelectionPadding, SelectionPadding);
	return true;
}

const FMatrix& UTextComponent::GetRenderWorldMatrix(const UCameraComponent* Camera) const
{
	if (!Camera) return GetWorldMatrix();

	// 빌보드 방향은 카메라를 따르되, 위치와 크기는 Attach 부모까지 반영한다.
	const FMatrix& WorldMatrix = GetWorldMatrix();
	const FVector WorldScale(
		WorldMatrix.GetAxis(0).Length(),
		WorldMatrix.GetAxis(1).Length(),
		WorldMatrix.GetAxis(2).Length());

	BillboardWorldMatrix = FMatrix::MakeScaleMatrix(WorldScale)
		* Camera->GetRelativeRotation().ToRotationMatrix()
		* FMatrix::MakeTranslationMatrix(WorldMatrix.GetOrigin());
	return BillboardWorldMatrix;
}

bool UTextComponent::BuildTextItem(const UCameraComponent* Camera, FWorldTextItem& OutItem) const
{
	if (!Camera || Text.empty()) return false;
	OutItem.Text = Text;
	OutItem.WorldMatrix = GetRenderWorldMatrix(Camera);
	return true;
}

void UTextComponent::Serialize(FArchive& Archive)
{
	Super::Serialize(Archive);
	Archive.SetString("Text", Text);
}

void UTextComponent::Deserialize(FArchive& Archive)
{
	Super::Deserialize(Archive);
	if (Archive.Contains("Text"))
	{
		Text = Archive.GetString("Text");
	}
}
