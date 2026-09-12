#include "pch.h"
#include "FEditor.h"

#include "Engine/Component/UCameraComponent.h"
#include "Engine/Actor/UActor.h"
#include "Editor/Window/UEditorWindow.h"

#include "Editor/Window/UConsoleWindow.h"
#include "Editor/Window/UPropertyWindow.h"
#include "Editor/Window/USceneWindow.h"

#include "Editor/Gizmo/UObjectAxisGizmo.h"
#include "Editor/Gizmo/UWorldAxisGizmo.h"
#include "Editor/Gizmo/UWorldGridGizmo.h"

#include "Editor/UGrid.h"

#include "Engine/Object/FObjectFactory.h"
#include "Engine/Log.h"

#include "Engine/Input/GInputManager.h"

#include "Editor/Picker/FObjectPicker.h"
#include "Editor/Picker/FGizmoPicker.h"
#include "Engine/Component/UWidgetComponent.h"

#include "Engine/Scene/GSceneManager.h"

#include "Engine/Scene/UScene.h"

#include "ImGui/imgui.h"


void FEditor::Initialize()
{
	EditorCamera = static_cast<UCameraComponent*>(SpawnObject(UCameraComponent::GetClass()));
	EditorCamera->SetRelativeLocation(FVector(-15.0f, -15.0f, 10.0f));
	EditorCamera->LookAt(FVector(0.0f, 0.0f, 0.0f));

	CameraController.SetCamera(EditorCamera);

	ObjectPicker = new FObjectPicker(this);
	GizmoPicker = new FGizmoPicker(this);

	InitializeGizmos();
	GizmoController = new FGizmoController(this);
	InitializeWindows();
	InitializeGrids();
}

void FEditor::InitializeGizmos()
{
	RegisterGizmo(UObjectAxisGizmo::GetClass());
	RegisterGizmo(UWorldAxisGizmo::GetClass());
	RegisterGizmo(UWorldGridGizmo::GetClass());
}

void FEditor::InitializeWindows()
{
	RegisterWindow(UConsoleWindow::GetClass());
	RegisterWindow(UPropertyWindow::GetClass());
	RegisterWindow(USceneWindow::GetClass());
}

void FEditor::InitializeGrids()
{
	RegisterGrid(UGrid::GetClass());
}

void FEditor::Tick(float DeltaTime)
{
	//CameraController.Tick(DeltaTime);
	GEngine& Engine = *GEngine::GetInstance();
	GInputManager& Input = *GInputManager::GetInstance();

	ImGuiIO& IO = ImGui::GetIO();
	bool bWantToCaptureMouse = IO.WantCaptureMouse;
	bool bWantToCaptureKeyboard = IO.WantCaptureKeyboard;

	float Time = Engine.GetTime();
	const bool bWasDragging = GizmoController->IsDragging();

	if (Input.ConsumeLeftClick() &&!bWasDragging &&!bWantToCaptureMouse &&!Input.GetKey(GInputManager::EI_RMOUSE))
	{
		int32 SelectedGizmo = GizmoPicker->Pick(ObjectAxisGizmo);
		//기즈모가 선택되면 드래그 시작
		if (SelectedGizmo != -1) {
			if (Input.GetKey(GInputManager::EI_LMOUSE))
				GizmoController->BeginDrag(SelectedGizmo);
		}
		//기즈모가 선택 안되면 오브젝트 선택
		else
		{
			UPrimitiveComponent* Selected = ObjectPicker->Pick();

			SetSelectedSceneComponent(Selected);
			if (Selected != nullptr) {
				UE_LOG("[{}] : [{}번째 오브젝트 선택]", Time, Selected->GetUUID());
			}
		}
	}

	const bool bGizmoOwnsInput = bWasDragging || GizmoController->IsDragging();

	GizmoController->Tick();
	if (bGizmoOwnsInput|| bWantToCaptureMouse)
	{
		// 카메라를 막는 동안 쌓인 회전 입력 폐기
		int32 DX, DY;
		Input.ConsumeRightDragDelta(DX, DY);
	}
	bool bRightClickDragging = Input.GetKey(GInputManager::EI_RMOUSE);
	bool bAllowCameraMouse = !bWantToCaptureMouse;
	bool bAllowCameraKeyboard = !bWantToCaptureKeyboard || (bAllowCameraMouse && bRightClickDragging);

	if (!bGizmoOwnsInput && bAllowCameraKeyboard && bAllowCameraMouse)
	{
		CameraController.Tick(DeltaTime);
	}
	const bool bSpacePressed = Input.ConsumeSpacePress();
	if (bSpacePressed && !IO.WantCaptureKeyboard)
	{
		GizmoController->ChangeMod();
	}
}

void FEditor::Release()
{
	delete GizmoController;
	GizmoController = nullptr;

	CameraController.SetCamera(nullptr);

	delete ObjectPicker;
	ObjectPicker = nullptr;

	delete GizmoPicker;
	GizmoPicker = nullptr;

	SelectedSceneComponent = nullptr;

	ReleaseGizmos();
	ReleaseWindows();
	ReleaseGrids();
}

void FEditor::ReleaseGizmos()
{
	ObjectAxisGizmo = nullptr;

	for (UGizmo* Gizmo : Gizmos)
	{
		delete Gizmo;
	}
	Gizmos.Empty();
}

void FEditor::ReleaseWindows()
{
	for (UEditorWindow* Window : Windows)
	{
		delete Window;
	}
	Windows.Empty();
}

void FEditor::ReleaseGrids()
{
	for (UGrid* Grid : Grids)
	{
		delete Grid;
	}
	Grids.Empty();
}

void FEditor::SpawnPrimitive(FClassType* PrimitiveType, int Count)
{
	UScene* CurrentScene = GetCurrentScene();

	for (int i = 0; i < Count; ++i)
	{
		UActor* Actor = CurrentScene->SpawnActor<UActor*>(UActor::GetClass());
		if (Actor->CreateComponent(PrimitiveType))
		{
			Actor->CreateComponent(UWidgetComponent::GetClass());
		}
	}
}

void FEditor::NewScene()
{
	// 똑같이 Scene을 불러오되, Deserialize 과정만 생략
	LoadScene("");
}

void FEditor::LoadScene(FStringView SceneName)
{
	SetSelectedSceneComponent(nullptr);
	GSceneManager* SceneManager = GSceneManager::GetInstance();
	FSceneType* SceneType = GetCurrentScene()->GetSceneType();

	SceneManager->LoadScene(SceneType, SceneName);
}

void FEditor::SaveScene(FStringView SceneName)
{
	GSceneManager* SceneManager = GSceneManager::GetInstance();
	SceneManager->SaveScene(SceneName);
}

UScene* FEditor::GetCurrentScene()
{
	GSceneManager* SceneManager = GSceneManager::GetInstance();
	return SceneManager->GetScene();
}

void FEditor::SetSelectedSceneComponent(USceneComponent* Component)
{
	SelectedSceneComponent = Component;
	if (GizmoController != nullptr)GizmoController->SetSelectedObject(Component);
}

void FEditor::DeleteSelectedSceneComponent()
{
	if (SelectedSceneComponent == nullptr) { return; }

	UScene* CurrentScene = GetCurrentScene();
	CurrentScene->Destroy(SelectedSceneComponent);

	SetSelectedSceneComponent(nullptr);
}

void FEditor::RegisterGizmo(FClassType* Type)
{
	UObject* Object = FObjectFactory::ConstructEditorObject(Type);
	UGizmo* Gizmo = static_cast<UGizmo*>(Object);

	Gizmo->Initialize(this);
	if (Gizmo->IsA(UObjectAxisGizmo::GetClass())) {
		SetObjectAxisGizmo(Gizmo);
	}
	Gizmos.Add(Gizmo);
}

void FEditor::RegisterWindow(FClassType* Type)
{
	UObject* Object = FObjectFactory::ConstructEditorObject(Type);
	UEditorWindow* Window = static_cast<UEditorWindow*>(Object);

	Window->Initialize(this);
	Windows.Add(Window);
}

void FEditor::SetObjectAxisGizmo(UGizmo* InGizmo)
{
	ObjectAxisGizmo = InGizmo;
}

UGizmo* FEditor::GetObjectAxisGizmo() const
{
	return ObjectAxisGizmo;
}

UObject* FEditor::SpawnObject(FClassType* Type)
{
	return FObjectFactory::ConstructEditorObject(Type);
}

void FEditor::RegisterGrid(FClassType* Type)
{
	UObject* Object = FObjectFactory::ConstructEditorObject(Type);
	UGrid* Grid = static_cast<UGrid*>(Object);

	Grid->Initialize(this);
	Grids.Add(Grid);
}
