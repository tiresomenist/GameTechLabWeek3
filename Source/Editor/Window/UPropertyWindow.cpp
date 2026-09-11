#include "pch.h"
#include "Editor/Util/ScaleEdit.h"
#include "UPropertyWindow.h"
#include "Core/Math/FVector.h"
#include "ImGui/imgui.h"
#include "Editor/FEditor.h"
#include "Core/Math/FQuaternion.h"
#include "ImGui/imgui_stdlib.h"
#include "Engine/Component/Primitive/UTextComponent.h"

void UPropertyWindow::GetSelectedValue()
{
	USceneComponent* NewComponent = Editor->GetSelectedSceneComponent();


	if (SelectedComponent != NewComponent)
	{
		SelectedComponent = NewComponent;
		bEditingRotation = false;
	}

	if (SelectedComponent != nullptr)
	{
		Translation = SelectedComponent->GetRelativeLocation();
		if (!bEditingRotation)
		{
			const FQuaternion& Quaternion = SelectedComponent->GetRelativeRotation();
			RotationDegree = FQuaternion::ToEuler(Quaternion) * (180.0f / PI);
		}
		OScale = SelectedComponent->GetRelativeScale3D();
	}
}

void UPropertyWindow::SetSelectedValue(bool bSetRotation)
{
	if (SelectedComponent != nullptr)
	{
		SelectedComponent->SetRelativeLocation(Translation);

		if (bSetRotation)
		{
			const FVector EulerRadians = RotationDegree * (PI / 180.0f);

			SelectedComponent->SetRelativeRotation(
				FQuaternion::FromEuler(EulerRadians)
			);
		}

		SelectedComponent->SetRelativeScale3D(OScale);
	}
}

bool UPropertyWindow::DrawRotationField(const char* ID, float& Degree, bool& bRotationActive)
{
	const bool bChanged = ImGui::DragFloat(ID,&Degree,0.1f,0.0f,0.0f,"%.3f");

	bRotationActive |= ImGui::IsItemActive();
	return bChanged;
}


void UPropertyWindow::DeleteSelected()
{
	Editor->DeleteSelectedSceneComponent();
	SelectedComponent = nullptr;
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
		ImGuiCond_Once
	);

	ImGui::SetNextWindowSize(
		ImVec2(WindowWidth, WindowHeight),
		ImGuiCond_Once
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
	bool bRotationFinished = false;
	float PreviousDegree = RotationDegree.X;

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
			constexpr ImGuiSliderFlags RotationFlags = ImGuiSliderFlags_WrapAround | ImGuiSliderFlags_AlwaysClamp;
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

			// 텍스트 컴포넌트 전용 속성
			if (SelectedComponent->IsA(UTextComponent::GetClass()))
			{
				DrawTextProperties(static_cast<UTextComponent*>(SelectedComponent));
			}
			if (ImGui::Button("Delete"))
			{
				DeleteSelected();
			}
		}
		ImGui::End();
	}
	bEditingRotation = bRotationActive;
	SetSelectedValue(bRotationChanged || bRotationFinished);
}

void UPropertyWindow::DrawTextProperties(UTextComponent* TextComponent)
{
	ImGui::SeparatorText("Text");

	// ImGui가 값을 직접 고치므로 복사본으로 받고, 바뀌었을 때만 Setter로 넣는다.
	// Setter는 같은 값이면 dirty를 켜지 않으므로 매 프레임 불려도 메시를 다시 만들지 않는다.

	// 문구: 여러 줄 입력. imgui_stdlib 덕분에 std::string을 바로 받는다 (UTF-8)
	FString Text = TextComponent->GetText();
	if (ImGui::InputTextMultiline("##Text", &Text, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4.0f)))
	{
		TextComponent->SetText(Text);
	}

	float Size = TextComponent->GetSize();
	if (ImGui::DragFloat("Size", &Size, 0.01f, 0.01f, 100.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
	{
		TextComponent->SetSize(Size);
	}

	const FVector4& Color = TextComponent->GetColor();
	float ColorValues[4] = { Color.X, Color.Y, Color.Z, Color.W };
	if (ImGui::ColorEdit4("Color", ColorValues))
	{
		TextComponent->SetColor(FVector4(ColorValues[0], ColorValues[1], ColorValues[2], ColorValues[3]));
	}

	// ETextAlign 순서(Left, Center, Right)와 같아야 한다
	const char* AlignNames[] = { "Left", "Center", "Right" };
	int Align = static_cast<int>(TextComponent->GetAlign());
	if (ImGui::Combo("Align", &Align, AlignNames, IM_ARRAYSIZE(AlignNames)))
	{
		TextComponent->SetAlign(static_cast<ETextAlign>(Align));
	}

	// ETextVAlign 순서(Top, Center, Bottom)와 같아야 한다
	const char* VAlignNames[] = { "Top", "Center", "Bottom" };
	int VAlign = static_cast<int>(TextComponent->GetVAlign());
	if (ImGui::Combo("Pivot", &VAlign, VAlignNames, IM_ARRAYSIZE(VAlignNames)))
	{
		TextComponent->SetVAlign(static_cast<ETextVAlign>(VAlign));
	}

	bool bBillboard = TextComponent->IsBillboard();
	if (ImGui::Checkbox("Billboard", &bBillboard))
	{
		TextComponent->SetBillboard(bBillboard);
	}
	ImGui::SameLine();
	bool bConstantScreenSize = TextComponent->IsConstantScreenSize();
	if (ImGui::Checkbox("Constant Screen Size", &bConstantScreenSize))
	{
		TextComponent->SetConstantScreenSize(bConstantScreenSize);
	}
}
