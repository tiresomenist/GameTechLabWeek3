#pragma once

#include <algorithm>
#include <cmath>

#include "Core/Container/FString.h"
#include "Core/Container/TArray.h"
#include "Engine/Object/FObjectFactory.h"
#include "Engine/Renderer/RenderUtil.h"
#include "Editor/Controller/FCameraController.h"
#include "Editor/Controller/FGizmoController.h"

//TESTCODE//
#include "Engine/Component/UCameraComponent.h"
#include "Engine/GEngine.h"
#include "Engine/FConsole.h"
#include "Engine/Renderer/FViewSettings.h"

#include "Engine/Renderer/FGrid.h"
#include "Core/Name/FName.h"


class USceneComponent;
class UCameraComponent;
class AActor;
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
	AActor* SelectedActor = nullptr;
	FViewSettings ViewSettings;
	FGrid Grid;
	TArray<UGizmo*> Gizmos;
	TArray<UEditorWindow*> Windows;
	TArray<UGrid*> Grids;
	UGizmo* ObjectAxisGizmo = nullptr;

	//예외처리용 초기화 여부
	bool bInitialized = false;
	bool bCanSaveEditorSettings = false;

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

	void SpawnStaticMesh(const FName& MeshKey, int Count);
	void SpawnComponent(FClassType* ComponentClass, int Count);
	void CreateEmptyActor();

	void NewScene();
	void LoadScene(FStringView SceneName);
	void LoadSceneFromPath(const std::filesystem::path& ScenePath);
	void SaveScene(FStringView SceneName);

	UScene* GetCurrentScene();
	UCameraComponent* GetEditorCamera() { return EditorCamera; }

	USceneComponent* GetSelectedSceneComponent() const { return SelectedSceneComponent; }
	AActor* GetSelectedActor() const { return SelectedActor; }
	
	//내부적으로 비트마스킹으로 처리해줌.
	bool IsShowingUUIDLabels() const { return ViewSettings.ShowFlags.IsEnabled(EEngineShowFlag::UUID); }
	void SetShowUUIDLabels(bool bShow) { ViewSettings.ShowFlags.SetEnabled(EEngineShowFlag::UUID, bShow); }
	bool IsShowingBoundingBoxes() const { return ViewSettings.ShowFlags.IsEnabled(EEngineShowFlag::Bounds); }
	void SetShowBoundingBoxes(bool bShow) { ViewSettings.ShowFlags.SetEnabled(EEngineShowFlag::Bounds, bShow); }
	EViewModeIndex GetViewMode() const { return ViewSettings.ViewMode; }
	void SetViewMode(EViewModeIndex InMode) { ViewSettings.ViewMode = InMode; }
	bool IsShowingPrimitives() const{return ViewSettings.ShowFlags.IsEnabled(EEngineShowFlag::Primitives);}
	void SetShowPrimitives(bool bShow){ViewSettings.ShowFlags.SetEnabled(EEngineShowFlag::Primitives, bShow);}
	bool IsShowingGrid() const{return ViewSettings.ShowFlags.IsEnabled(EEngineShowFlag::Grid);}
	void SetShowGrid(bool bShow){ViewSettings.ShowFlags.SetEnabled(EEngineShowFlag::Grid, bShow);}
	bool IsShowingWorldAxis() const { return ViewSettings.ShowFlags.IsEnabled(EEngineShowFlag::WorldAxis); }
	void SetShowWorldAxis(bool bShow) { ViewSettings.ShowFlags.SetEnabled(EEngineShowFlag::WorldAxis, bShow); }
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
	void SetSelectedActor(AActor* Actor);

	void RemoveSelectedComponent();
	void DeleteSelectedActor();

	void RegisterGizmo(FClassType* Type);
	void RegisterWindow(FClassType* Type);
	void RegisterGrid(FClassType* Type);

	void LoadEditorSetting();

	void SaveEditorSetting();

	const TArray<UGizmo*>& GetGizmos() const { return Gizmos; }
	const TArray<UEditorWindow*>& GetWindows() const { return Windows; }
	const TArray<UGrid*>& GetGrids() const { return Grids; }

	//TEST CODE//
	FVector GetCameraLocation() { return GetEditorCamera()->GetRelativeLocation(); }
	void SetCameraLocation(FVector NewCameraLocation) { EditorCamera->SetRelativeLocation(NewCameraLocation); }
	FVector GetCameraRotationDegree()
	{
		const FRotator& CameraRotation = GetEditorCamera()->GetRelativeRotator();
		return CameraRotation.ToEulerDegrees();
	}
	void SetCameraRotationDegree(const FVector& NewRotationDegree)
	{
		GetEditorCamera()->SetRelativeRotation(FRotator::FromEulerDegrees(NewRotationDegree));
	}
	float GetCameraFOV() { return GetEditorCamera()->GetFOV() * 180.0f / PI; }
	void SetCameraFOV(float NewFOV) { EditorCamera->SetFOVByDegree(NewFOV); }
	
	void SpawnPrimitives(FClassType* ClassType, uint32 num) { GEngine::GetInstance()->GetConsole()->Append(std::format("Make {}, {} times",ClassType->Name,num)); }
	
	void SetObjectAxisGizmo(UGizmo* InGizmo);
	UGizmo* GetObjectAxisGizmo()const;

	UObject* SpawnObject(FClassType* Type);

public:
	friend TArray<FPrimitiveRenderData> RenderUtil::GetRenderList(FEditor* Editor, UScene* Scene);
	friend TArray<FPrimitiveRenderData> RenderUtil::GetGizmoList(FEditor* Editor, UScene* Scene);
};
