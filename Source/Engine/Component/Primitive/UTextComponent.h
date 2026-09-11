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
	ETextVAlign GetVAlign() const { return VAlign; }
	bool IsBillboard() const { return bBillboard; }
	bool IsConstantScreenSize() const { return bConstantScreenSize; }

	// 값이 실제로 바뀔 때만 메시를 dirty로 표시한다.
	// (ImGui처럼 매 프레임 같은 값을 넣어 줘도 메시를 다시 만들지 않게)
	void SetText(const FString& InText);
	void SetFontName(const FString& InFontName);
	void SetSize(float InSize);
	void SetColor(const FVector4& InColor);
	void SetAlign(ETextAlign InAlign);
	void SetVAlign(ETextVAlign InVAlign);

	// 아래 둘은 메시가 아니라 렌더 행렬에만 영향을 주므로 dirty를 켜지 않는다
	void SetBillboard(bool bInBillboard) { bBillboard = bInBillboard; }
	void SetConstantScreenSize(bool bInConstant) { bConstantScreenSize = bInConstant; }

	virtual FPrimitiveRenderData CreateRenderData(bool bSelected = false) const override;
	virtual bool GetLocalBounds(FVector& OutMin, FVector& OutMax) const override;

	virtual void Serialize(FArchive& Archive) override;
	virtual void Deserialize(FArchive& Archive) override;

	// 빌보드면 회전을 카메라 회전으로, 화면 크기 고정이면 스케일에 거리 보정을 곱한다.
	// 컴포넌트의 실제 회전/스케일 값은 건드리지 않는다 (저장, 기즈모, 프로퍼티 편집값 유지)
	virtual const FMatrix& GetRenderWorldMatrix(const UCameraComponent* Camera) const override;

private:
	FTextStyle MakeStyle() const;

	// 원본: 사용자가 정하고 씬에 저장되는 값
	FString Text = "Text";
	FString FontName = "Pretendard";   // 포인터가 아니라 이름: 파일에 저장할 수 있고, 소유자는 GResourceManager
	float Size = 1.0f;
	FVector4 Color = FVector4(1, 1, 1, 1);
	ETextAlign Align = ETextAlign::Center;
	ETextVAlign VAlign = ETextVAlign::Top;   // 기존에 저장한 씬의 배치가 바뀌지 않도록 Top이 기본
	bool bBillboard = false;
	bool bConstantScreenSize = false;

	// 화면 크기 고정의 기준: 원근은 카메라 앞으로 이 거리에 있을 때, 직교는 OrthoHeight가 이 값일 때 원래 크기
	static constexpr float ReferenceDistance = 10.0f;
	static constexpr float ReferenceOrthoHeight = 10.0f;

	// 파생: 원본에서 언제든 다시 만들 수 있는 렌더 캐시.
	// const인 CreateRenderData에서 갱신하므로 mutable (USceneComponent::CachedWorldMatrix와 같은 이유)
	mutable FTextMesh Mesh;
	mutable bool bMeshDirty = true;

	// GetRenderWorldMatrix가 참조를 돌려주기 위한 저장소 (카메라가 바뀔 때마다 다시 계산)
	mutable FMatrix RenderWorldMatrix;
};
