#pragma once
#include "Editor/Window/UEditorWindow.h"
#include "Core/Math/Matrix.h"
#include "Core/Math/FQuaternion.h"
#include "Core/Container/FString.h"

class USceneComponent;
struct FClassType;

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
	TArray<FClassType*> AddableComponentClasses;
	FClassType* SelectedAddComponentClass = nullptr;
	TArray<FString> SpawnableMeshKeys;
	FString SelectedMeshKey;
public:
	virtual void Initialize(FEditor* InEditor) override;

	void GetSelectedValue();
	void SetSelectedValue(bool bSetRotation);
	void RemoveSelectedComponent();
	void DeleteSelectedActor();

	bool DrawRotationField(const char* ID, float& Degree, bool& bRotationActive);

	void Render(float DeltaTime) override;
};

