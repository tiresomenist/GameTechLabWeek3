#pragma once

#include <algorithm>
#include <cmath>

#include "Core/Container/TArray.h"
#include "Engine/Object/FObjectFactory.h"
#include "Engine/Renderer/RenderUtil.h"
#include "Editor/Controller/FCameraController.h"
#include "Editor/Controller/FGizmoController.h"

//TESTCODE//
#include "Engine/Component/UCameraComponent.h"
#include "Engine/GEngine.h"
#include "Engine/FConsole.h"

class USceneComponent;
class UCameraComponent;
class UEditorWindow;
class UGizmo;
class UGrid;
class FObjectPicker;
class FGizmoPicker;

class FEditor
{
private:
	// 현재 선택된 SceneComponent
	UCameraComponent* EditorCamera = nullptr;
	FCameraController CameraController;
	FObjectPicker* ObjectPicker = nullptr;
	FGizmoPicker* GizmoPicker = nullptr;
	FGizmoController* GizmoController = nullptr;

	USceneComponent* SelectedSceneComponent = nullptr;
	bool bShowUUIDLabels = true;
	bool bShowBoundingBoxes = true;
	EViewModeIndex ViewMode = EViewModeIndex::VMI_Unlit;
	FGrid Grid;
	TArray<UGizmo*> Gizmos;
	TArray<UEditorWindow*> Windows;
	TArray<UGrid*> Grids;
	UGizmo* ObjectAxisGizmo = nullptr;

	void InitializeGizmos();
	void InitializeWindows();
	void InitializeGrids();

	void ReleaseGizmos();
	void ReleaseWindows();
	void ReleaseGrids();

public:

	void Initialize();

	void Tick(float DeltaTime);

	void Release();

	void SpawnPrimitive(FClassType* PrimitiveType, int Count);

	void NewScene();
	void LoadScene(FStringView SceneName);
	void SaveScene(FStringView SceneName);

	UScene* GetCurrentScene();
	UCameraComponent* GetEditorCamera() { return EditorCamera; }

	USceneComponent* GetSelectedSceneComponent() const { return SelectedSceneComponent; }
	bool IsShowingUUIDLabels() const { return bShowUUIDLabels; }
	void SetShowUUIDLabels(bool bShow) { bShowUUIDLabels = bShow; }
	bool IsShowingBoundingBoxes() const { return bShowBoundingBoxes; }
	void SetShowBoundingBoxes(bool bShow) { bShowBoundingBoxes = bShow; }
	EViewModeIndex GetViewMode() const { return ViewMode; }
	void SetViewMode(EViewModeIndex InMode) { ViewMode = InMode; }
	const FGrid& GetGrid() const { return Grid; }
	void SetGridInterval(float InInterval)
	{
		if (std::isfinite(InInterval))
		{
			Grid.Interval = std::clamp(InInterval, FGrid::MinInterval, FGrid::MaxInterval);
		}
	}
	int32 GetActiveGizmoAxis() const { return GizmoController ? GizmoController->GetActiveAxis() : -1; }
	void SetSelectedSceneComponent(USceneComponent* Component);

	void DeleteSelectedSceneComponent();

	void RegisterGizmo(FClassType* Type);
	void RegisterWindow(FClassType* Type);
	void RegisterGrid(FClassType* Type);

	const TArray<UGizmo*>& GetGizmos() const { return Gizmos; }
	const TArray<UEditorWindow*>& GetWindows() const { return Windows; }
	const TArray<UGrid*>& GetGrids() const { return Grids; }

	//TEST CODE//
	FVector GetCameraLocation() { return GetEditorCamera()->GetRelativeLocation(); }
	void SetCameraLocation(FVector NewCameraLocation) { EditorCamera->SetRelativeLocation(NewCameraLocation); }
	FVector GetCameraRotationDegree()
	{
		const FQuaternion& CameraRotation = GetEditorCamera()->GetRelativeRotation();
		return FQuaternion::ToEuler(CameraRotation) * (180.0f / PI);
	}
	void SetCameraRotationDegree(const FVector& NewRotationDegree)
	{
		const FVector EulerRadian = NewRotationDegree * (PI / 180.0f);

		const FQuaternion CameraRotation = FQuaternion::FromEuler(EulerRadian);

		EditorCamera->SetRelativeRotation(CameraRotation);
	}
	float GetCameraFOV() { return GetEditorCamera()->GetFOV() * 180.0f / PI; }
	void SetCameraFOV(float NewFOV) { EditorCamera->SetFOVByDegree(NewFOV); }
	
	void SpawnPrimitives(FClassType* ClassType, uint32 num) { GEngine::GetInstance()->GetConsole()->Append(std::format("Make {}, {} times", ClassType->Name.ToString(), num)); }
	
	void SetObjectAxisGizmo(UGizmo* InGizmo);
	UGizmo* GetObjectAxisGizmo()const;

	UObject* SpawnObject(FClassType* Type);

public:
	friend TArray<FPrimitiveRenderData> RenderUtil::GetRenderList(FEditor* Editor, UScene* Scene);
	friend TArray<FPrimitiveRenderData> RenderUtil::GetGizmoList(FEditor* Editor, UScene* Scene);
};
