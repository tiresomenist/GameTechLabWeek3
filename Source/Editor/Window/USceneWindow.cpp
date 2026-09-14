#include "pch.h"
#include "USceneWindow.h"

#include <algorithm>
#include <cmath>

#include "Editor/FEditor.h"
#include "Engine/FConsole.h"
#include "Engine/GEngine.h"
#include "Core/Container/TArray.h"
#include "Engine/Object/FClassType.h"
#include "Engine/Component/Primitive/USphereComponent.h"
#include "Engine/Component/Primitive/UCubeComponent.h"
#include "Engine/Component/Primitive/UTriangleComponent.h"
#include "Engine/Component/Primitive/UPlaneComponent.h"
#include "Engine/Component/Primitive/UPepeComponent.h"
#include "Engine/Component/Primitive/UOctopusComponent.h"
#include "Engine/Component/Primitive/UTextComponent.h"
#include "Engine/Component/UCameraComponent.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_stdlib.h"
#include "Engine/Input/GInputManager.h"
#include "Engine/Memory/GAllocator.h"

#include "Engine/Scene/GSceneManager.h"

void USceneWindow::SpawnPrimitive() 
{
	Editor->SpawnPrimitive(SelectedClass, NumberOfSpawn);
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
	Editor->LoadScene(SceneName);
}
void USceneWindow::Initialize(FEditor* Editor)
{
	UEditorWindow::Initialize(Editor);

	Spawnables.Add(USphereComponent::GetClass());
	Spawnables.Add(UCubeComponent::GetClass());
	Spawnables.Add(UPlaneComponent::GetClass());
	Spawnables.Add(UTriangleComponent::GetClass());
	Spawnables.Add(UPepeComponent::GetClass());
	Spawnables.Add(UOctopusComponent::GetClass());
	Spawnables.Add(UTextComponent::GetClass());

	SelectedClass = *Spawnables.begin();

	SceneName.reserve(128);
}


void USceneWindow::Render(float DeltaTime)
{
	UCameraComponent* EditorCamera = Editor->GetEditorCamera();

	CameraLocation = Editor->GetCameraLocation();
	if (!bEditingCameraRotation)
	{
		const FVector Forward = EditorCamera->GetForward();
		const float HorizontalLength = std::hypot(Forward.X, Forward.Y);

		// Editor camera는 항상 수평을 유지하므로 Roll은 사용하지 않는다.
		// 일반 ToEuler() 대신 시선 방향에서 Pitch/Yaw를 직접 구해야
		// CameraController의 월드 Yaw + 로컬 Pitch 회전 방식과 값이 일치한다.
		CameraRotationDegree.X = 0.0f;
		CameraRotationDegree.Y = std::atan2(-Forward.Z, HorizontalLength) * (180.0f / PI);
		CameraRotationDegree.Z = std::atan2(Forward.Y, Forward.X) * (180.0f / PI);
	}
	FOV = Editor->GetCameraFOV();
	MoveSpeed = Editor->GetCameraMoveSpeed();

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
		ImGuiCond_Once
	);

	ImGui::SetNextWindowSize(
		ImVec2(WindowWidth, WindowHeight),
		ImGuiCond_Once
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
		if (ImGui::BeginCombo("Primitive", SelectedClass->Name.c_str(), ImGuiComboFlags_HeightSmall))
		{
			for (FClassType* ClassType : Spawnables)
			{
				bool bSelected = (SelectedClass == ClassType);

				if (ImGui::Selectable(ClassType->Name.c_str(), bSelected))
				{
					SelectedClass = ClassType;
				}

				if (bSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::PopItemWidth();
		if (ImGui::Button("Spawn"))
		{
			SpawnPrimitive();
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
			LoadScene();
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
		if (ImGui::DragFloat("FOV", &FOV, 1.0f, MinFOV, MaxFOV))
		{
			FOV = std::clamp(FOV, MinFOV, MaxFOV);
			Editor->SetCameraFOV(FOV); // 무조건 업데이트 시키면 라디안 값 FOV가 0에 가까워지므로 조건부로
		}
		ImGui::PushItemWidth(WideItemWidth); // Item 너비 설정
		if (ImGui::DragFloat("MoveSpeed", &MoveSpeed, 0.1f, MinMoveSpeed, MaxMoveSpeed))
		{
			MoveSpeed = std::clamp(MoveSpeed, MinMoveSpeed, MaxMoveSpeed);
			Editor->SetCameraMoveSpeed(MoveSpeed);
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
		bRotationChanged |= ImGui::DragFloat("##cameraRZ", &CameraRotationDegree.Z, 0.1f, -180.0f, 180.0f, "%.3f", YawFlags);
		bRotationActive |= ImGui::IsItemActive();
		bRotationFinished |= ImGui::IsItemDeactivatedAfterEdit();
		DrawItemBottomLine(IM_COL32(20, 30, 255, 255), 2.0f);
		ImGui::SameLine();
		ImGui::Text("Camera Rotation");
		if (bRotationChanged || bRotationFinished)
		{
			const float PitchRadian = std::clamp(CameraRotationDegree.Y, -89.0f, 89.0f) * (PI / 180.0f);
			const float YawRadian = CameraRotationDegree.Z * (PI / 180.0f);

			const FQuaternion YawRotation =
				FQuaternion::FromAxisAngle(FVector(0.0f, 0.0f, 1.0f), YawRadian);
			const FQuaternion PitchRotation =
				FQuaternion::FromAxisAngle(FVector(0.0f, 1.0f, 0.0f), PitchRadian);

			// CameraController와 동일하게 월드 Z축 Yaw를 먼저 구성하고,
			// 카메라의 로컬 Y축 Pitch가 되도록 오른쪽에 곱한다.
			EditorCamera->SetRelativeRotation(YawRotation * PitchRotation);
		}
		bEditingCameraRotation = bRotationActive;
		ImGui::PopItemWidth();
		//ImGui::PopStyleVar();
	}
	Editor->SetCameraLocation(CameraLocation);
	ImGui::End();
}
