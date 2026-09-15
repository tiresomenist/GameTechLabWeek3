#include "pch.h"
#include "Editor/Util/ScaleEdit.h"
#include "UPropertyWindow.h"
#include "Core/Math/FVector.h"
#include "ImGui/imgui.h"
#include "Editor/FEditor.h"
#include "Core/Math/FQuaternion.h"
#include "Engine/Component/Primitive/UFlameComponent.h"
#include <algorithm>
#include <cmath>
namespace
{
	bool IsSameRotation(const FQuaternion& Left, const FQuaternion& Right)
	{
		FQuaternion A = Left;
		FQuaternion B = Right;

		A.Normalize();
		B.Normalize();

		const auto Square = [](double Value){return Value * Value;};

		// 같은 부호로 표현된 쿼터니언의 차이 계산함
		const double SameSignDistance =
			Square(static_cast<double>(A.X) - B.X) +
			Square(static_cast<double>(A.Y) - B.Y) +
			Square(static_cast<double>(A.Z) - B.Z) +
			Square(static_cast<double>(A.W) - B.W);

		// Q와 -Q가 같은 자세인 경우까지 비교함
		const double OppositeSignDistance =
			Square(static_cast<double>(A.X) + B.X) +
			Square(static_cast<double>(A.Y) + B.Y) +
			Square(static_cast<double>(A.Z) + B.Z) +
			Square(static_cast<double>(A.W) + B.W);

		// 정규화된 쿼터니언 성분 차이의 제곱 허용치
		constexpr double ToleranceSquared = 1.0e-12;

		return (std::min)(SameSignDistance,	OppositeSignDistance) <= ToleranceSquared;
	}
}

void UPropertyWindow::GetSelectedValue()
{
	USceneComponent* NewComponent = Editor->GetSelectedSceneComponent();


	if (SelectedComponent != NewComponent)
	{
		SelectedComponent = NewComponent;
		bEditingRotation = false;
		bHasSyncedRotation = false;
	}

	if (!SelectedComponent)
	{
		bEditingRotation = false;
		bHasSyncedRotation = false;
		return;
	}
	Translation = SelectedComponent->GetRelativeLocation();
	OScale = SelectedComponent->GetRelativeScale3D();

	const FQuaternion CurrentRotation =	SelectedComponent->GetRelativeRotation();

	// 최초 선택 시 현재 자세로 UI 초기화함
	if (!bHasSyncedRotation)
	{
		RotationDegree =FQuaternion::ToEuler(CurrentRotation) * (180.0f / PI);
		LastSyncedRotation = CurrentRotation;
		bHasSyncedRotation = true;
		return;
	}

	// 숫자 편집 중에는 입력 중인 Euler 값을 유지함
	if (bEditingRotation) { return; }
	// 기즈모·코드 등으로 실제 자세가 변경된 경우에만 동기화함
	if (!IsSameRotation(CurrentRotation,LastSyncedRotation))
	{
		RotationDegree =FQuaternion::ToEuler(CurrentRotation) * (180.0f / PI);
		LastSyncedRotation = CurrentRotation;
	}
}

void UPropertyWindow::SetSelectedValue(bool bSetRotation)
{
	if (!SelectedComponent) { return; }

	SelectedComponent->SetRelativeLocation(Translation);

	if (bSetRotation)
	{
		const FVector EulerRadians = RotationDegree * (PI / 180.0f);
		const FQuaternion NewRotation =	FQuaternion::FromEuler(EulerRadians);
		SelectedComponent->SetRelativeRotation(NewRotation);

		// 컴포넌트에서 정규화까지 적용된 실제 값을 기록함
		LastSyncedRotation = SelectedComponent->GetRelativeRotation();
		bHasSyncedRotation = true;
	}
	SelectedComponent->SetRelativeScale3D(OScale);
}

bool UPropertyWindow::DrawRotationField(const char* ID, float& Degree, bool& bRotationActive)
{
	const float PreviousDegree = Degree;
	const bool bChanged = ImGui::DragFloat(ID,&Degree,0.1f,0.0f,0.0f,"%.3f");

	bRotationActive |= ImGui::IsItemActive();
	if (bChanged && !std::isfinite(Degree))
	{
		Degree = PreviousDegree;
		return false;
	}
	return bChanged;
}


void UPropertyWindow::DeleteSelected()
{
	Editor->DeleteSelectedSceneComponent();
	SelectedComponent = nullptr;

	bEditingRotation = false;
	bHasSyncedRotation = false;
}

void UPropertyWindow::Render(float DeltaTime)
{
	const ImGuiViewport* Viewport = ImGui::GetMainViewport();
	const ImVec2 WorkPosition = Viewport->WorkPos; // 메뉴창을 제외한 제일 왼쪽 위 위치
	const ImVec2 WorkSize = Viewport->WorkSize;    // 메뉴창을 제외한 Imgui를 띄울 수 있는 공간

	// 전체 프로그램 창 크기에 대한 비율
	constexpr float WindowWidthRatio = 0.35f;
	constexpr float WindowHeightRatio = 0.25f;

	float WindowWidth = WindowWidthRatio * 1000.0f;
	float WindowHeight = WindowHeightRatio * 1000.0f;

	ImVec2 NewPosition = WorkPosition;
	NewPosition.x += WorkSize.x * 0.42f;

	ImGui::SetNextWindowPos(
		NewPosition,
		ImGuiCond_FirstUseEver
	);

	ImGui::SetNextWindowSize(
		ImVec2(WindowWidth, WindowHeight),
		ImGuiCond_FirstUseEver
	);

	ImVec2 Available = ImGui::GetContentRegionAvail();
	//float Scale = std::clamp(WindowWidth / 400.0f, 0.1f, 5.0f);
	//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3.0f * Scale, 2.0f * Scale)); // 버튼 안쪽 여백 증가
	const ImGuiStyle& Style = ImGui::GetStyle();
	ImVec2 ItemSpacing = Style.ItemSpacing; // 아이템간 패딩 값
	float ButtonWidth = Available.x * 0.2f; // Button, DragFloat
	float ComboWidth = Available.x * 0.3f;

	GetSelectedValue();
	bool bRotationChanged = false;
	bool bRotationActive = false;
	
	if (SelectedComponent != nullptr)
	{
		ImGui::Begin("Property Window");
		{
			ImGui::PushItemWidth(ButtonWidth);
			ImGui::DragFloat("##translationX", &Translation.X, SnapSize);
			DrawItemBottomLine(IM_COL32(255, 40, 40, 255), 2.0f);
			ImGui::SameLine();
			ImGui::DragFloat("##translationY", &Translation.Y, SnapSize);
			DrawItemBottomLine(IM_COL32(40, 255, 40, 255), 2.0f);
			ImGui::SameLine();
			ImGui::DragFloat("##translationZ", &Translation.Z, SnapSize);
			DrawItemBottomLine(IM_COL32(20, 30, 255, 255), 2.0f);
			ImGui::SameLine();
			ImGui::Text("Translation");
			bRotationChanged |= DrawRotationField("##rotationR", RotationDegree.X, bRotationActive);
			DrawItemBottomLine(IM_COL32(255, 40, 40, 255), 2.0f);
			ImGui::SameLine();
			bRotationChanged |= DrawRotationField("##rotationP", RotationDegree.Y, bRotationActive);
			DrawItemBottomLine(IM_COL32(40, 255, 40, 255), 2.0f);
			ImGui::SameLine();
			bRotationChanged |= DrawRotationField("##rotationY", RotationDegree.Z, bRotationActive);
			DrawItemBottomLine(IM_COL32(20, 30, 255, 255), 2.0f);
			ImGui::SameLine();
			ImGui::Text("Rotation");
			const FVector BeforeX = OScale;
            float EditedX = OScale.X;
            if (ImGui::DragFloat("##scaleX", &EditedX, 0.001f))
            {
                FVector Result;
                if (ApplyScaleEdit(BeforeX, 0, EditedX, bScaleLock, Result)) OScale = Result;
            }
            DrawItemBottomLine(IM_COL32(255, 40, 40, 255), 2.0f);
			ImGui::SameLine();
			const FVector BeforeY = OScale;
            float EditedY = OScale.Y;
            if (ImGui::DragFloat("##scaleY", &EditedY, 0.001f))
            {
                FVector Result;
                if (ApplyScaleEdit(BeforeY, 1, EditedY, bScaleLock, Result)) OScale = Result;
            }
            DrawItemBottomLine(IM_COL32(40, 255, 40, 255), 2.0f);
			ImGui::SameLine();
			const FVector BeforeZ = OScale;
            float EditedZ = OScale.Z;
            if (ImGui::DragFloat("##scaleZ", &EditedZ, 0.001f))
            {
                FVector Result;
                if (ApplyScaleEdit(BeforeZ, 2, EditedZ, bScaleLock, Result)) OScale = Result;
            }
            DrawItemBottomLine(IM_COL32(20, 30, 255, 255), 2.0f);
			ImGui::SameLine();
			ImGui::Text("Scale");
			ImGui::PopItemWidth();
			//ImGui::PopStyleVar();
			ImGui::PushItemWidth(ComboWidth);
			char SnapPrev[32];
			snprintf(SnapPrev, sizeof(SnapPrev), "%g", SnapSizeList[SelectedSnapIndex]);
			if (ImGui::BeginCombo("SnapSize", SnapPrev))
			{
				for (int i = 0; i < SnapSizeList.Num(); i++)
				{
					bool bSelected = (SelectedSnapIndex == i);

					char ItemName[32];
					snprintf(ItemName, sizeof(ItemName), "%g", SnapSizeList[i]);

					if (ImGui::Selectable(ItemName, bSelected))
					{
						SelectedSnapIndex = i;
					}

					if (bSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
					SnapSize = SnapSizeList[SelectedSnapIndex];
				}
				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::Checkbox("Scale Lock", &bScaleLock);
            if (SelectedComponent->IsA(UFlameComponent::GetClass()) &&
                ImGui::CollapsingHeader("SubUV", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto* Flame = static_cast<UFlameComponent*>(SelectedComponent);
                int Grid[2] = { Flame->GetColumns(), Flame->GetRows() };
                if (ImGui::InputInt2("Columns / Rows", Grid))
                    Flame->SetAtlasGrid(Grid[0], Grid[1]);

                int FrameCount = Flame->GetFrameCount();
                if (ImGui::InputInt("Frame Count", &FrameCount))
                    Flame->SetAtlasGrid(Flame->GetColumns(), Flame->GetRows(), FrameCount);

                float FPS = Flame->GetFramesPerSecond();
                if (ImGui::DragFloat("FPS", &FPS, 1.0f, 0.0f, 240.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp))
                    Flame->SetFramesPerSecond(FPS);
                float Rate = Flame->GetPlayRate();
                if (ImGui::DragFloat("Play Rate", &Rate, 0.05f, 0.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
                    Flame->SetPlayRate(Rate);

                bool bLoop = Flame->IsLooping();
                if (ImGui::Checkbox("Loop", &bLoop)) Flame->SetLooping(bLoop);
                ImGui::SameLine();
                bool bPlaying = Flame->IsPlaying();
                if (ImGui::Checkbox("Playing", &bPlaying)) Flame->SetPlaying(bPlaying);
                ImGui::SameLine();
                if (ImGui::Button("Restart")) Flame->Restart();

                // 프레임을 직접 선택하면 정지하여 해당 칸을 확인함
                int Frame = Flame->GetCurrentFrame();
                if (ImGui::SliderInt("Frame", &Frame, 0, Flame->GetFrameCount() - 1))
                {
                    Flame->SetCurrentFrame(Frame);
                    Flame->SetPlaying(false);
                }
            }
			if (ImGui::Button("Delete"))
			{
				DeleteSelected();
			}
		}
		ImGui::End();
	}
	bEditingRotation = bRotationActive;
	SetSelectedValue(bRotationChanged);
	bEditingRotation = SelectedComponent != nullptr && bRotationActive;
}
