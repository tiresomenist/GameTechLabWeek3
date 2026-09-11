#pragma once

#include "Engine/Component/Primitive/UPrimitiveComponent.h"
#include "Engine/Renderer/FTextMesh.h"

// 월드에 글자를 띄우는 컴포넌트.
// 원본 값(Text, FontName, Size, Color, Align)이 바뀌면 bMeshDirty를 켜고,
// 렌더 직전에 FTextMesh를 다시 만든다.
class UTextComponent : public UPrimitiveComponent
{
	UCLASS(UTextComponent, "TextComponent", UPrimitiveComponent)

public:
	const FString& GetText() const { return Text; }
	const FString& GetFontName() const { return FontName; }
	float GetSize() const { return Size; }
	const FVector4& GetColor() const { return Color; }
	ETextAlign GetAlign() const { return Align; }

	// 값이 실제로 바뀔 때만 메시를 dirty로 표시한다.
	// (ImGui처럼 매 프레임 같은 값을 넣어 줘도 메시를 다시 만들지 않게)
	void SetText(const FString& InText);
	void SetFontName(const FString& InFontName);
	void SetSize(float InSize);
	void SetColor(const FVector4& InColor);
	void SetAlign(ETextAlign InAlign);

	virtual FPrimitiveRenderData CreateRenderData(bool bSelected = false) const override;
	virtual bool GetLocalBounds(FVector& OutMin, FVector& OutMax) const override;

	virtual void Serialize(FArchive& Archive) override;
	virtual void Deserialize(FArchive& Archive) override;

private:
	FTextStyle MakeStyle() const;

	// 원본: 사용자가 정하고 씬에 저장되는 값
	FString Text = "Text";
	FString FontName = "Pretendard";   // 포인터가 아니라 이름: 파일에 저장할 수 있고, 소유자는 GResourceManager
	float Size = 1.0f;
	FVector4 Color = FVector4(1, 1, 1, 1);
	ETextAlign Align = ETextAlign::Center;

	// 파생: 원본에서 언제든 다시 만들 수 있는 렌더 캐시.
	// const인 CreateRenderData에서 갱신하므로 mutable (USceneComponent::CachedWorldMatrix와 같은 이유)
	mutable FTextMesh Mesh;
	mutable bool bMeshDirty = true;
};
