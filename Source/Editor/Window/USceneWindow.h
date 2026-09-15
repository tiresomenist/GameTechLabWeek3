#pragma once

#include "UEditorWindow.h"
#include "Engine/Object/FClassType.h"
#include "Core/Math/Matrix.h"
#include "Core/Container/FString.h"
#include "Core/Math/FQuaternion.h"

class FEditor;

class USceneWindow : public UEditorWindow
{
    UCLASS(USceneWindow, "SceneWindow", UEditorWindow)

private:
	uint32 NumberOfSpawn = 1;
	uint32 Step = 1;
	FString SceneName{"NewScene"};
	bool bOrthogonal = false;
	TArray<FString> SpawnableMeshKeys;
	FString SelectedMeshKey;
	/* Camera Info */
	float FOV = 90.0f;
	float MaxFOV = 175.0f;
	float MinFOV = 5.0f;
	FVector CameraLocation = { 0.0f, 0.0f, 0.0f };
	FVector CameraRotationDegree;
	bool bEditingCameraRotation = false;

	TArray<FClassType*> SpecialComponentClasses;
	FClassType* SelectedSpecialComponentClass = nullptr;
	/*             */
public:
	void SpawnStaticMesh();
	void SpawnSpecialComponent();
	void SpawnEmptyActor();
	void NewScene();
	void SaveScene();
	void LoadScene();

	virtual void Initialize(FEditor* InEditor) override;
	void Render(float DeltaTime) override;


};
