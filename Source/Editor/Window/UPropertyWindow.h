#pragma once
#include "Editor/Window/UEditorWindow.h"
#include "Core/Math/Matrix.h"
#include "Core/Math/FQuaternion.h"
#include "Core/Container/FString.h"

class USceneComponent;
class UTextComponent;

class UPropertyWindow : public UEditorWindow
{
    UCLASS(UPropertyWindow, "PropertyWindow", UEditorWindow)

private:
	USceneComponent* SelectedComponent = nullptr;
	FVector Translation;
	FVector RotationDegree;
	FVector OScale;
	FQuaternion RotationDragStart;
	float RotationDragStartDegree = 0.0f;
	float RotationDegreeAtDragStart = 0.0f;
	int RotationDragAxis = -1;
	bool bEditingRotation = false;
	float SnapSize = 0.001f;
	int SelectedSnapIndex = 0;
	TArray<float> SnapSizeList = {0.001f, 0.01f, 0.1f, 1.0f, 5.0f};
	bool bScaleLock = false;
public:

	void GetSelectedValue();
	void SetSelectedValue(bool bSetRotation);
	void DeleteSelected();

	bool DrawRotationField(const char* ID, float& Degree, bool& bRotationActive);

	// 선택한 컴포넌트가 UTextComponent일 때만 그리는 전용 속성 (문구, 크기, 색, 정렬)
	void DrawTextProperties(UTextComponent* TextComponent);

	void Render(float DeltaTime) override;
};

