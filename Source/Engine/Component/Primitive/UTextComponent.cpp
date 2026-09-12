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

	BillboardWorldMatrix = FMatrix::MakeScaleMatrix(GetRelativeScale3D())
		* Camera->GetRelativeRotation().ToRotationMatrix()
		* FMatrix::MakeTranslationMatrix(GetRelativeLocation());
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
