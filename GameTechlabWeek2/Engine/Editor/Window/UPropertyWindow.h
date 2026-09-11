#pragma once
#include "Engine/Editor/Window/UEditorWindow.h"
#include "../../../Matrix.h"
#include "../../../FQuaternion.h"
#include "../../../Container/FString.h"

class USceneComponent;

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

	void Render(float DeltaTime) override;
};

