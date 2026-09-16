#include "pch.h"
#include "USceneWindow.h"

#include <algorithm>
#include <cmath>

#include "Editor/FEditor.h"
#include "Engine/FConsole.h"
#include "Engine/GEngine.h"
#include "Core/Container/TArray.h"
#include "Engine/Component/UCameraComponent.h"
#include "Engine/Component/Primitive/UFlipbookComponent.h"
#include "Engine/Component/Primitive/UTextComponent.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_stdlib.h"
#include "Engine/Input/GInputManager.h"
#include "Engine/Memory/GAllocator.h"
#include "Engine/Renderer/FViewSettings.h"
#include "Engine/Scene/GSceneManager.h"
#include "Core/Util/File.h"

#include "SolarSystem.h"

void USceneWindow::SpawnStaticMesh()
{
	Editor->SpawnStaticMesh(SelectedMeshKey, NumberOfSpawn);
}

void USceneWindow::SpawnSpecialComponent()
{
	Editor->SpawnComponent(SelectedSpecialComponentClass, NumberOfSpawn);
}

void USceneWindow::SpawnEmptyActor()
{
	Editor->CreateEmptyActor();
}

void USceneWindow::NewScene()
{
	Editor->NewScene();
}
void USceneWindow::SaveScene()
{
	Editor->SaveScene(SceneName);
}
void USceneWindow::LoadScene()
{
	// imgui_impl_win32가 메인 뷰포트에 HWND를 넣어두므로 그걸 대화상자 owner로 사용
	const HWND Owner = static_cast<HWND>(ImGui::GetMainViewport()->PlatformHandleRaw);

	const std::optional<std::filesystem::path> ScenePath = File::OpenFileDialog(Owner, EFileDialogType::Json, "Scenes");
	if (!ScenePath)
	{
		return; // 취소
	}

	// 이후 Save Scene이 같은 이름으로 저장되도록 이름 칸도 갱신
	SceneName = ScenePath->stem().string();
	Editor->LoadSceneFromPath(*ScenePath);
}
void USceneWindow::Initialize(FEditor* Editor)
{
	UEditorWindow::Initialize(Editor);

	SpecialComponentClasses.Add(UTextComponent::GetClass());
	SpecialComponentClasses.Add(UFlipbookComponent::GetClass());

	SelectedSpecialComponentClass = *SpecialComponentClasses.begin();

	SpawnableMeshKeys.Add(FString("Sphere"));
	SpawnableMeshKeys.Add(FString("Cube"));
	SpawnableMeshKeys.Add(FString("Plane"));
	SpawnableMeshKeys.Add(FString("Triangle"));
	SpawnableMeshKeys.Add(FString("Pepe"));
	SpawnableMeshKeys.Add(FString("Octopus"));
	SelectedMeshKey = *SpawnableMeshKeys.begin();

	SceneName.reserve(128);
}


void USceneWindow::Render(float DeltaTime)
{
	UCameraComponent* EditorCamera = Editor->GetEditorCamera();

	CameraLocation = Editor->GetCameraLocation();
	if (!bEditingCameraRotation)
	{
		CameraRotationDegree = Editor->GetCameraRotationDegree();
	}
	FOV = Editor->GetCameraFOV();

	const ImGuiViewport* Viewport = ImGui::GetMainViewport();
	const ImVec2 WorkPosition = Viewport->WorkPos; // 메뉴창을 제외한 제일 왼쪽 위 위치
	const ImVec2 WorkSize = Viewport->WorkSize;    // 메뉴창을 제외한 Imgui를 띄울 수 있는 공간

	// 전체 프로그램 창 크기에 대한 비율
	constexpr float WindowWidthRatio = 0.42f;
	constexpr float WindowHeightRatio = 0.36f;

	float WindowWidth = WorkSize.x * WindowWidthRatio;
	float WindowHeight = WorkSize.y * WindowHeightRatio;

	ImGui::SetNextWindowPos(
		WorkPosition,
		ImGuiCond_FirstUseEver
	);

	ImGui::SetNextWindowSize(
		ImVec2(WindowWidth, WindowHeight),
		ImGuiCond_FirstUseEver
	);
	
	ImVec2 Available = ImGui::GetContentRegionAvail();
	//float Scale = std::clamp(WindowWidth / 400.0f, 0.1f, 5.0f);
	//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(3.0f * Scale , 2.0f * Scale)); // 버튼 안쪽 여백 증가
	const ImGuiStyle& Style = ImGui::GetStyle();
	ImVec2 ItemSpacing = Style.ItemSpacing; // 아이템간 패딩 값

	float ButtonWidth = Available.x * 0.2f; // Button, DragFloat
	float WideItemWidth = ButtonWidth * 3.0f + ItemSpacing.x * 2.0f; // FOV, NumberOfSpawn

	size_t AllocationBytes = GAllocator::GetTotalAllocationBytes();
	size_t AllocationCount = GAllocator::GetTotalAllocationCount();
	
	ImGui::Begin("Scene Control Panel", nullptr, ImGuiWindowFlags_HorizontalScrollbar);
	{
		float MilliSeconds = DeltaTime * 1000;
		ImGui::Text("PEPE Engine");
		ImGui::Text("FPS %.00f (%.00f ms)", 1000 / MilliSeconds, MilliSeconds);
		ImGui::Separator();
		ImGui::Text("UObject Heap Memory 사용량: %zu바이트", AllocationBytes);
		ImGui::Text("UObject Heap Memory 객체 수: %zu개", AllocationCount);
		ImGui::Separator();

		ImGui::PushItemWidth(WideItemWidth);
		if (ImGui::BeginCombo("Static Mesh", SelectedMeshKey.c_str(), ImGuiComboFlags_HeightSmall))
		{
			for (const FString& MeshKey : SpawnableMeshKeys)
			{
				bool bSelected = (SelectedMeshKey == MeshKey);

				if (ImGui::Selectable(MeshKey.c_str(), bSelected))
				{
					SelectedMeshKey = MeshKey;
				}

				if (bSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::PopItemWidth();
		if (ImGui::Button("Spawn Static Mesh"))
		{
			SpawnStaticMesh();
		}
		ImGui::SameLine();
		ImGui::PushItemWidth(200);
		if (ImGui::InputScalar(
			"Number Of Spawn",
			ImGuiDataType_U32,
			&NumberOfSpawn,
			&Step))
		{
			NumberOfSpawn = std::clamp(NumberOfSpawn, 1u, 20u);
		}
		ImGui::PopItemWidth();

		ImGui::PushItemWidth(WideItemWidth);
		if (ImGui::BeginCombo(
			"Special Component",
			SelectedSpecialComponentClass->Name.c_str(),
			ImGuiComboFlags_HeightSmall))
		{
			for (FClassType* ComponentClass : SpecialComponentClasses)
			{
				const bool bSelected = SelectedSpecialComponentClass == ComponentClass;
				if (ImGui::Selectable(ComponentClass->Name.c_str(), bSelected))
				{
					SelectedSpecialComponentClass = ComponentClass;
				}
				if (bSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::PopItemWidth();
		if (ImGui::Button("Spawn Special Component"))
		{
			SpawnSpecialComponent();
		}
		ImGui::SameLine();
		if (ImGui::Button("Create Empty Actor"))
		{
			SpawnEmptyActor();
		}
		ImGui::Separator();
		ImGui::PushItemWidth(WideItemWidth);

		ImGui::InputText("Scene Name", &SceneName);

		ImGui::PopItemWidth();
		if(ImGui::Button("New Scene"))
		{
			NewScene();
		}
		if(ImGui::Button("Save Scene"))
		{
			SaveScene();
		}
		if(ImGui::Button("Load Scene"))
		{
			bRequestLoadDialog = true;
		}
		ImGui::Separator();
		bool bShowUUIDLabels = Editor->IsShowingUUIDLabels();
		if (ImGui::Checkbox("Show UUID", &bShowUUIDLabels))
		{
			Editor->SetShowUUIDLabels(bShowUUIDLabels);
		}
		bool bShowBoundingBoxes = Editor->IsShowingBoundingBoxes();
		if (ImGui::Checkbox("Show Bounding Boxes", &bShowBoundingBoxes))
		{
			Editor->SetShowBoundingBoxes(bShowBoundingBoxes);
		}
		bool bShowPrimitives = Editor->IsShowingPrimitives();
		if (ImGui::Checkbox("Show Primitives", &bShowPrimitives))
		{
			Editor->SetShowPrimitives(bShowPrimitives);
		}
		bool bShowGrid = Editor->IsShowingGrid();
		if (ImGui::Checkbox("Show Grid", &bShowGrid))
		{
			Editor->SetShowGrid(bShowGrid);
		}
		ImGui::PushItemWidth(WideItemWidth);
		float GridInterval = Editor->GetGrid().Interval;
		if (ImGui::DragFloat("Grid Spacing", &GridInterval, 0.1f,
			FGrid::MinInterval, FGrid::MaxInterval, "%.2f", ImGuiSliderFlags_AlwaysClamp))
		{
			Editor->SetGridInterval(GridInterval);
		}

		const EViewModeIndex CurrentViewMode = Editor->GetViewMode();
		const char* Preview = "Unknown";
		for (const FViewModeEntry& Entry : ViewModeEntries)
		{
			if (Entry.Mode == CurrentViewMode)
			{
				Preview = Entry.Name;
				break;
			}
		}
		if (ImGui::BeginCombo("View Mode", Preview))
		{
			for (const FViewModeEntry& Entry : ViewModeEntries)
			{
				const bool bSelected = Entry.Mode == CurrentViewMode;
				if (ImGui::Selectable(Entry.Name, bSelected))
				{
					Editor->SetViewMode(Entry.Mode);
				}
				if (bSelected) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::PopItemWidth();
		ImGui::Checkbox("Orthogonal", &bOrthogonal);

		EditorCamera->SetIsPerspective(!bOrthogonal);

		ImGui::PushItemWidth(WideItemWidth); // Item 너비 설정
		// 카메라의 현재 이동 속도를 조회하고 UI 변경 시 즉시 적용함
		float CameraMoveSpeed = EditorCamera->GetMoveSpeed();
		if (ImGui::DragFloat("Camera Move Speed", &CameraMoveSpeed, 0.1f,
			0.1f, 100.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
		{
			if (std::isfinite(CameraMoveSpeed))
			{
				EditorCamera->SetMoveSpeed(std::clamp(CameraMoveSpeed, 0.1f, 100.0f));
			}
		}

		if (ImGui::DragFloat("FOV", &FOV, 1.0f, MinFOV, MaxFOV))
		{
			FOV = std::clamp(FOV, MinFOV, MaxFOV);
			Editor->SetCameraFOV(FOV); // 무조건 업데이트 시키면 라디안 값 FOV가 0에 가까워지므로 조건부로
		}
		ImGui::PopItemWidth();
		ImGui::PushItemWidth(ButtonWidth); // Item 너비 설정
		ImGui::DragFloat("##cameraX", &CameraLocation.X, 0.1f);
		DrawItemBottomLine(IM_COL32(255, 40, 40, 255), 2.0f);
		ImGui::SameLine();
		ImGui::DragFloat("##cameraY", &CameraLocation.Y, 0.1f);
		DrawItemBottomLine(IM_COL32(40, 255, 40, 255), 2.0f);
		ImGui::SameLine();
		ImGui::DragFloat("##cameraZ", &CameraLocation.Z, 0.1f);
		DrawItemBottomLine(IM_COL32(20, 30, 255, 255), 2.0f);
		ImGui::SameLine();
		ImGui::Text("Camera Location");
		bool bRotationChanged = false;
		bool bRotationActive = false;
		bool bRotationFinished = false;
		constexpr ImGuiSliderFlags PitchFlags = ImGuiSliderFlags_AlwaysClamp;
		constexpr ImGuiSliderFlags YawFlags = ImGuiSliderFlags_WrapAround | ImGuiSliderFlags_AlwaysClamp;

		// ConstrainEditorRotation()이 Roll을 제거하므로 수정할 수 없는 값으로 표시한다.
		ImGui::BeginDisabled();
		ImGui::DragFloat("##cameraRX", &CameraRotationDegree.X, 0.1f, -180.0f, 180.0f, "%.3f");
		ImGui::EndDisabled();
		bRotationActive |= ImGui::IsItemActive();
		bRotationFinished |= ImGui::IsItemDeactivatedAfterEdit();
		DrawItemBottomLine(IM_COL32(255, 40, 40, 255), 2.0f);
		ImGui::SameLine();
		bRotationChanged |= ImGui::DragFloat("##cameraRY", &CameraRotationDegree.Y, 0.1f, -89.0f, 89.0f, "%.3f", PitchFlags);
		bRotationActive |= ImGui::IsItemActive();
		bRotationFinished |= ImGui::IsItemDeactivatedAfterEdit();
		DrawItemBottomLine(IM_COL32(40, 255, 40, 255), 2.0f);
		ImGui::SameLine();
		bRotationChanged |= ImGui::DragFloat("##cameraRZ", &CameraRotationDegree.Z, 0.1f, 0.0f, 0.0f, "%.3f");
		bRotationActive |= ImGui::IsItemActive();
		bRotationFinished |= ImGui::IsItemDeactivatedAfterEdit();
		DrawItemBottomLine(IM_COL32(20, 30, 255, 255), 2.0f);
		ImGui::SameLine();
		ImGui::Text("Camera Rotation");
		if (bRotationChanged || bRotationFinished)
		{
			CameraRotationDegree.X = 0.0f;

			// 카메라 Pitch를 도 단위로 제한함
			CameraRotationDegree.Y = std::clamp(CameraRotationDegree.Y,	-89.0f,	89.0f);

			// 도 단위 입력값을 FRotator Setter로 전달함
			Editor->SetCameraRotationDegree(CameraRotationDegree);
		}
		bEditingCameraRotation = bRotationActive;
		ImGui::PopItemWidth();
		//ImGui::PopStyleVar();
		ImGui::Separator();
		if (ImGui::Button("SpawnSolarSystem"))
		{
			SpawnSolarSystem(Editor->GetCurrentScene());
		}
	}
	Editor->SetCameraLocation(CameraLocation);
	ImGui::End();

	if (bRequestLoadDialog)
	{
		bRequestLoadDialog = false;
		LoadScene();
	}
}
