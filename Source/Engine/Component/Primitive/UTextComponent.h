#pragma once

#include "Engine/Component/Primitive/UPrimitiveComponent.h"
#include "Engine/Renderer/Text/FWorldTextItem.h"

class UCameraComponent;
class FArchive;

// 문자열을 월드에 빌보드로 표시하고, 문자열 전체 AABB로 선택할 수 있는 Primitive다.
class UTextComponent : public UPrimitiveComponent
{
	UCLASS(UTextComponent, "Text", UPrimitiveComponent)

public:
	const FString& GetText() const { return Text; }
	void SetText(const FString& InText) { Text = InText; }

	virtual FPrimitiveRenderData CreateRenderData(bool bSelected = false) const override { return {}; }
	virtual bool GetLocalBounds(FVector& OutMin, FVector& OutMax) const override;
	virtual bool IsAABBOnlyPickable() const override { return true; }
	virtual const FMatrix& GetRenderWorldMatrix(const UCameraComponent* Camera) const override;

	bool BuildTextItem(const UCameraComponent* Camera, FWorldTextItem& OutItem) const;

	virtual void Serialize(FArchive& Archive) override;
	virtual void Deserialize(FArchive& Archive) override;

private:
	static constexpr float SelectionPadding = 0.05f;

	FString Text = "한글 텍스트";
	mutable FMatrix BillboardWorldMatrix;
};
