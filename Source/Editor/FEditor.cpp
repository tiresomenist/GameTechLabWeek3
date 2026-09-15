#include "pch.h"
#include "FEditor.h"

#include "Engine/Component/UCameraComponent.h"
#include "Engine/Actor/AActor.h"
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
#include "Engine/Component/UStaticMeshComponent.h"
#include "Engine/Component/UStaticMeshComponent.h"

#include "Engine/Scene/GSceneManager.h"

#include "Engine/Scene/UScene.h"

#include "ImGui/imgui.h"
#include "Core/Util/File.h"

#include <charconv>
#include <cmath>
#include <exception>
#include <filesystem>
#include <format>
#include <stdexcept>
#include <system_error>

namespace
{
	constexpr char EditorSettingsFileName[] = "editor.ini";

	FStringView TrimIniWhitespace(FStringView Text)
	{
		const auto First = Text.find_first_not_of(" \t\r\n");
		if (First == FStringView::npos)
		{
			return {};
		}
		const auto Last = Text.find_last_not_of(" \t\r\n");
		return Text.substr(First, Last - First + 1);
	}

	bool TryReadIniValue(FStringView Text, FStringView Section, FStringView Key, FStringView& OutValue)
	{
		// UTF-8 BOM이 존재하는 경우 삭제
		if (Text.starts_with("\xEF\xBB\xBF"))
		{
			Text.remove_prefix(3);
		}

		FStringView CurrentSection;
		FStringView FoundValue;
		bool bFound = false;

		while (!Text.empty())
		{
			// 현재 줄과 이후 내용을 분리함
			const auto Newline = Text.find('\n');
			FStringView Line;

			if (Newline == FStringView::npos)
			{
				Line = Text;
				Text = {};
			}
			else
			{
				Line = Text.substr(0, Newline);
				Text.remove_prefix(Newline + 1);
			}

			Line = TrimIniWhitespace(Line);

			// 빈 줄과 전체 줄 주석을 무시함
			if (Line.empty()||Line.front() == ';'||Line.front() == '#')
			{
				continue;
			}

			// 현재 섹션을 갱신함
			if (Line.front() == '[')
			{
				CurrentSection = {};

				if (Line.size() >= 2 && Line.back() == ']')
				{
					CurrentSection = TrimIniWhitespace(Line.substr(1, Line.size() - 2));
				}

				continue;
			}

			if (CurrentSection != Section)
			{
				continue;
			}

			// 키와 값을 첫 번째 등호를 기준으로 분리함
			const auto EqualPosition = Line.find('=');

			if (EqualPosition == FStringView::npos)
			{
				continue;
			}

			const FStringView LineKey = TrimIniWhitespace(Line.substr(0, EqualPosition));

			if (LineKey != Key)
			{
				continue;
			}

			FoundValue = TrimIniWhitespace(Line.substr(EqualPosition + 1));

			// 동일한 키가 중복되면 마지막 값을 사용함
			bFound = true;
		}

		if (!bFound)
		{
			return false;
		}

		OutValue = FoundValue;
		return true;
	}

	bool TryReadIniFloat(FStringView Text,FStringView Section,FStringView Key,float& OutValue)
	{
		FStringView ValueText;

		if (!TryReadIniValue(Text, Section, Key, ValueText))
		{
			return false;
		}

		float ParsedValue = 0.0f;
		bool bValid = false;

		if (!ValueText.empty())
		{
			const char* Begin = ValueText.data();
			const char* End = Begin + ValueText.size();

			const auto Result = std::from_chars(Begin, End,ParsedValue,std::chars_format::general);

			bValid = (Result.ec==std::errc{})&&(Result.ptr == End)&&(std::isfinite(ParsedValue));
		}

		if (!bValid)
		{
			UE_LOG("설정 숫자 형식 오류: [{}] {}={}",Section,Key,ValueText);
			return false;
		}

		OutValue = ParsedValue;
		return true;
	}

	bool TryReadIniBool(FStringView Text, FStringView Section, FStringView Key, bool& OutValue)
	{
		FStringView ValueText;

		if (!TryReadIniValue(Text, Section, Key, ValueText))
		{
			return false;
		}

		if (ValueText == "true" || ValueText == "1")
		{
			OutValue = true;
			return true;
		}

		if (ValueText == "false" || ValueText == "0")
		{
			OutValue = false;
			return true;
		}

		UE_LOG("설정 Bool 형식 오류: [{}] {}={}",Section,Key,ValueText);

		return false;
	}
	const char* GetViewModeName(EViewModeIndex Mode)
	{
		for (const FViewModeEntry& Entry : ViewModeEntries)
		{
			if (Entry.Mode == Mode)
			{
				return Entry.Name;
			}
		}
		return nullptr;
	}

	bool TryParseViewMode(FStringView Name, EViewModeIndex& OutMode)
	{
		for (const FViewModeEntry& Entry : ViewModeEntries)
		{
			if (Name == FStringView(Entry.Name))
			{
				OutMode = Entry.Mode;
				return true;
			}
		}
		return false;
	}

	struct FShowFlagIniEntry
	{
		const char* Key;
		EEngineShowFlag Flag;
	};

	// INI 키와 내부 ShowFlag의 대응 관계
	constexpr FShowFlagIniEntry ShowFlagIniEntries[] =
	{
		{ "ShowUUID",       EEngineShowFlag::UUID },
		{ "ShowPrimitives", EEngineShowFlag::Primitives },
		{ "ShowGrid",       EEngineShowFlag::Grid },
		{ "ShowBounds",     EEngineShowFlag::Bounds },
	};
}

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
	LoadEditorSetting();

	bInitialized = true;
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
	if (bInitialized) {
		bInitialized = false;
		try
		{
			SaveEditorSetting();
		}
		catch (const std::exception& Exception)
		{
			UE_LOG("에디터 설정 저장 실패:{}", Exception.what());
		}
	}
	
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

void FEditor::SpawnPrimitive(const FString& MeshKey, int Count)
{
	UScene* CurrentScene = GetCurrentScene();

	for (int i = 0; i < Count; ++i)
	{
		AActor* Actor = CurrentScene->SpawnActor<AActor*>(AActor::GetClass());
		auto* StaticMesh = static_cast<UStaticMeshComponent*>(
			Actor->CreateComponent(UStaticMeshComponent::GetClass()));

		StaticMesh->SetStaticMesh(MeshKey);
		Actor->CreateComponent(UWidgetComponent::GetClass());
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


//이하 두 함수에서 editor.ini을 읽어서 설정을 저장하거나 불러온다.
void FEditor::LoadEditorSetting()
{
	// 정상적인 로드 여부가 결정되기 전까지 저장을 금지함
	bCanSaveEditorSettings = false;

	if (!EditorCamera) { return; }

	try
	{
		// 파일이 없으면 기본값으로 시작하고 신규 저장을 허용함
		if (!std::filesystem::exists(EditorSettingsFileName))
		{
			bCanSaveEditorSettings = true;
			return;
		}

		const FString FileText = File::ReadText(EditorSettingsFileName);

		// 불러오기가 실패할때 기본값으로 설정하도록 초기화
		float LoadedMoveSpeed = EditorCamera->GetMoveSpeed();
		float LoadedGridInterval = GetGrid().Interval;
		FViewSettings LoadedViewSettings = ViewSettings;

		float ParsedFloat = 0.0f;

		// 카메라 이동 속도를 읽고 양수 여부를 검사함
		if (TryReadIniFloat(FileText,"Camera","MoveSpeed",ParsedFloat))
		{
			if (ParsedFloat > 0.0f)
			{
				LoadedMoveSpeed = ParsedFloat;
			}
			else
			{
				UE_LOG("MoveSpeed 범위 오류. 기존 값 유지: {}",LoadedMoveSpeed);
			}
		}

		// 그리드 간격을 읽고 허용 범위를 검사함
		if (TryReadIniFloat(FileText,"Grid","Interval",ParsedFloat))
		{
			if (ParsedFloat >= FGrid::MinInterval && ParsedFloat <= FGrid::MaxInterval)
			{
				LoadedGridInterval = ParsedFloat;
			}
			else
			{
				UE_LOG("Grid Interval 범위 오류. 기존 값 유지: {}",LoadedGridInterval);
			}
		}

		// 문자열로 저장된 뷰 모드를 복원함
		FStringView ViewModeText;

		if (TryReadIniValue(FileText,"Viewport","ViewMode",ViewModeText))
		{
			EViewModeIndex ParsedMode = LoadedViewSettings.ViewMode;

			if (TryParseViewMode(ViewModeText, ParsedMode))
			{
				LoadedViewSettings.ViewMode = ParsedMode;
			}
			else
			{
				UE_LOG("알 수 없는 ViewMode: {}. 기존 값 유지함.",ViewModeText);
			}
		}

		// 파일에 존재하는 정상적인 ShowFlag 항목만 변경함
		for (const FShowFlagIniEntry& Entry : ShowFlagIniEntries)
		{
			bool ParsedBool = false;

			if (TryReadIniBool(FileText,"Viewport",Entry.Key,ParsedBool))
			{
				LoadedViewSettings.ShowFlags.SetEnabled(Entry.Flag,ParsedBool);
			}
		}

		// 모든 항목의 해석 완료 후 실제 설정에 적용함
		EditorCamera->SetMoveSpeed(LoadedMoveSpeed);
		SetGridInterval(LoadedGridInterval);
		ViewSettings = LoadedViewSettings;

		bCanSaveEditorSettings = true;
	}
	catch (const std::exception& Exception)
	{
		UE_LOG("에디터 설정 로드 실패: {}. 기존 설정과 파일을 유지함.",Exception.what());
	}
}

void FEditor::SaveEditorSetting() {
	if (!EditorCamera||!bCanSaveEditorSettings) { return; }
	
	const float MoveSpeed = EditorCamera->GetMoveSpeed();
	const float GridInterval = GetGrid().Interval;
	const char* ViewModeName = GetViewModeName(ViewSettings.ViewMode);
	if (!std::isfinite(MoveSpeed) || MoveSpeed <= 0.0f)
	{
		throw std::runtime_error("잘못된 카메라 이동속도를 저장할 수 없음");
	}

	if (!std::isfinite(GridInterval) || GridInterval < FGrid::MinInterval || GridInterval > FGrid::MaxInterval)
	{
		throw std::runtime_error("잘못된 그리드 간격을 저장할 수 없음");
	}

	if (!ViewModeName)
	{
		throw std::runtime_error("알수 없는 뷰모드를 저장할 수 없음");
	}
	//// 현재 설정값을 INI 형식으로 구성함
	FString FileText = std::format(
		"[Camera]\n"
		"MoveSpeed={}\n"
		"\n"
		"[Grid]\n"
		"Interval={}\n"
		"\n"
		"[Viewport]\n"
		"ViewMode={}\n", 
		EditorCamera->GetMoveSpeed(),
		GetGrid().Interval, 
		ViewModeName
		//ShowFlags는 어떻게 처리할지 고민좀 해봐야됨
	);

	for (const FShowFlagIniEntry& Entry : ShowFlagIniEntries)
	{
		const bool bEnabled = ViewSettings.ShowFlags.IsEnabled(Entry.Flag);
		FileText += std::format("{}={}\n",Entry.Key,bEnabled ? "true" : "false");
	}

	File::WriteText("editor.ini", FileText);
}
