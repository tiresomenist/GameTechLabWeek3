#include "pch.h"
#include "Editor/Util/ScaleEdit.h"
#include "UPropertyWindow.h"
#include "Core/Math/FVector.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_stdlib.h"
#include "Editor/FEditor.h"
#include "Core/Math/FQuaternion.h"
#include "Engine/Actor/AActor.h"
#include "Engine/Component/Primitive/UPrimitiveComponent.h"
#include "Engine/Component/Primitive/UFlipbookComponent.h"
#include "Engine/Component/Primitive/UTextComponent.h"
#include "Engine/Component/UStaticMeshComponent.h"
#include "Engine/Component/UWidgetComponent.h"
#include "Core/Math/FRotator.h"


namespace
{
	UStaticMeshComponent* FindStaticMeshComponent(AActor* Actor)
	{
		if (Actor == nullptr) return nullptr;

		for (UActorComponent* Component : Actor->GetComponents())
		{
			if (Component->IsA(UStaticMeshComponent::GetClass()))
			{
				return static_cast<UStaticMeshComponent*>(Component);
			}
		}

		return nullptr;
	}

	void EnsurePrimitiveWidget(AActor* Actor)
	{
		for (UActorComponent* Component : Actor->GetComponents())
		{
			if (Component->IsA(UWidgetComponent::GetClass())) return;
		}

		Actor->CreateComponent(UWidgetComponent::GetClass());
	}
}

void UPropertyWindow::Initialize(FEditor* InEditor)
{
	UEditorWindow::Initialize(InEditor);

	AddableComponentClasses.Add(UStaticMeshComponent::GetClass());
	AddableComponentClasses.Add(UTextComponent::GetClass());
	AddableComponentClasses.Add(UFlipbookComponent::GetClass());
	SelectedAddComponentClass = *AddableComponentClasses.begin();

	SpawnableMeshKeys.Add(FString("Sphere"));
	SpawnableMeshKeys.Add(FString("Cube"));
	SpawnableMeshKeys.Add(FString("Plane"));
	SpawnableMeshKeys.Add(FString("Triangle"));
	SpawnableMeshKeys.Add(FString("Pepe"));
	SpawnableMeshKeys.Add(FString("Octopus"));
	SelectedMeshKey = *SpawnableMeshKeys.begin();
}

void UPropertyWindow::GetSelectedValue()
{
	USceneComponent* NewComponent = Editor->GetSelectedSceneComponent();
	if (SelectedComponent != NewComponent)
	{
		SelectedComponent = NewComponent;
		bEditingRotation = false;
	}

	if (!SelectedComponent)
	{
		bEditingRotation = false;
		return;
	}
	Translation = SelectedComponent->GetRelativeLocation();
	OScale = SelectedComponent->GetRelativeScale3D();
	// 입력 중에는 창의 임시 값을 유지함
	if (!bEditingRotation)
	{
		RotationDegree = SelectedComponent->GetRelativeRotator();
	}
}

void UPropertyWindow::SetSelectedValue(bool bSetRotation)
{
	if (!SelectedComponent) return;
	SelectedComponent->SetRelativeLocation(Translation);
	if (bSetRotation)
	{
		SelectedComponent->SetRelativeRotation(RotationDegree);
	}
	SelectedComponent->SetRelativeScale3D(OScale);
}

bool UPropertyWindow::DrawRotationField(const char* ID, float& Degree, bool& bRotationActive)
{
	const float PreviousDegree = Degree;
	const bool bChanged = ImGui::DragFloat(ID, &Degree, 0.1f, 0.0f, 0.0f, "%.3f");

	bRotationActive |= ImGui::IsItemActive();
	if (bChanged && !std::isfinite(Degree))
	{
		Degree = PreviousDegree;
		return false;
	}
	return bChanged;
}


void UPropertyWindow::RemoveSelectedComponent()
{
	Editor->RemoveSelectedComponent();
	SelectedComponent = nullptr;
}

void UPropertyWindow::DeleteSelectedActor()
{
	Editor->DeleteSelectedActor();
	SelectedComponent = nullptr;
	bEditingRotation = false;
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
	bool bRotationFinished = false;
	float PreviousDegree = RotationDegree.Roll;

	AActor* SelectedActor = Editor->GetSelectedActor();
	if (SelectedActor != nullptr)
	{
		ImGui::Begin("Property Window");
		{
			const FString ActorName = SelectedActor->GetName().ToString();
			ImGui::Text("Actor: %s", ActorName.c_str());

			if (ImGui::CollapsingHeader("Components", ImGuiTreeNodeFlags_DefaultOpen))
			{
				for (UActorComponent* Component : SelectedActor->GetComponents())
				{
					const FString ComponentLabel = std::format(
						"{}##{}", Component->GetName().ToString(), Component->GetUUID());
					if (!Component->IsA(USceneComponent::GetClass()))
					{
						ImGui::TextDisabled("%s", ComponentLabel.c_str());
						continue;
					}

					USceneComponent* SceneComponent = static_cast<USceneComponent*>(Component);
					if (ImGui::Selectable(ComponentLabel.c_str(), SelectedComponent == SceneComponent))
					{
						Editor->SetSelectedSceneComponent(SceneComponent);
						GetSelectedValue();
					}
				}
			}

			if (ImGui::CollapsingHeader("Add Component", ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (ImGui::BeginCombo("Component Type", SelectedAddComponentClass->Name.c_str()))
				{
					for (FClassType* ComponentClass : AddableComponentClasses)
					{
						const bool bSelected = SelectedAddComponentClass == ComponentClass;
						if (ImGui::Selectable(ComponentClass->Name.c_str(), bSelected))
						{
							SelectedAddComponentClass = ComponentClass;
						}
						if (bSelected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				const bool bAddingStaticMesh =
					SelectedAddComponentClass == UStaticMeshComponent::GetClass();
				if (bAddingStaticMesh &&
					ImGui::BeginCombo("Mesh", SelectedMeshKey.c_str()))
				{
					for (const FString& MeshKey : SpawnableMeshKeys)
					{
						const bool bSelected = SelectedMeshKey == MeshKey;
						if (ImGui::Selectable(MeshKey.c_str(), bSelected))
						{
							SelectedMeshKey = MeshKey;
						}
						if (bSelected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				if (ImGui::Button(bAddingStaticMesh && FindStaticMeshComponent(SelectedActor)
					? "Apply Static Mesh" : "Add Component"))
				{
					UActorComponent* AddedComponent = nullptr;
					if (bAddingStaticMesh)
					{
						if (UStaticMeshComponent* StaticMesh = FindStaticMeshComponent(SelectedActor))
						{
							StaticMesh->SetStaticMesh(SelectedMeshKey);
							AddedComponent = StaticMesh;
						}
						else
						{
							AddedComponent = SelectedActor->CreateComponent(SelectedAddComponentClass);
							if (AddedComponent != nullptr)
							{
								static_cast<UStaticMeshComponent*>(AddedComponent)->SetStaticMesh(SelectedMeshKey);
							}
						}
					}
					else
					{
						AddedComponent = SelectedActor->CreateComponent(SelectedAddComponentClass);
					}

					if (AddedComponent != nullptr && AddedComponent->IsA(UPrimitiveComponent::GetClass()))
					{
						EnsurePrimitiveWidget(SelectedActor);
					}
					if (AddedComponent != nullptr && AddedComponent->IsA(USceneComponent::GetClass()))
					{
						Editor->SetSelectedSceneComponent(static_cast<USceneComponent*>(AddedComponent));
					}
				}
			}

			if (SelectedComponent != nullptr && SelectedComponent->GetOwner() == SelectedActor)
			{
				if (ImGui::Button("Remove Selected Component"))
				{
					RemoveSelectedComponent();
				}
				ImGui::SameLine();
			}
			if (ImGui::Button("Delete Actor"))
			{
				DeleteSelectedActor();
			}
			ImGui::Separator();

			if (SelectedComponent != nullptr)
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
				bRotationChanged |= DrawRotationField("##rotationR", RotationDegree.Roll, bRotationActive);
				DrawItemBottomLine(IM_COL32(255, 40, 40, 255), 2.0f);
				ImGui::SameLine();
				bRotationChanged |= DrawRotationField("##rotationP", RotationDegree.Pitch, bRotationActive);
				DrawItemBottomLine(IM_COL32(40, 255, 40, 255), 2.0f);
				ImGui::SameLine();
				bRotationChanged |= DrawRotationField("##rotationY", RotationDegree.Yaw, bRotationActive);
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
				if (SelectedComponent->IsA(UFlipbookComponent::GetClass()) &&
					ImGui::CollapsingHeader("SubUV", ImGuiTreeNodeFlags_DefaultOpen))
				{
					auto* Flame = static_cast<UFlipbookComponent*>(SelectedComponent);
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
				if (SelectedComponent->IsA(UStaticMeshComponent::GetClass()) &&
					ImGui::CollapsingHeader("Static Mesh", ImGuiTreeNodeFlags_DefaultOpen))
				{
					auto* MeshComp = static_cast<UStaticMeshComponent*>(SelectedComponent);

					ImGui::Text("Mesh Key: %s", MeshComp->GetStaticMeshKey().c_str());

					std::string CurrentTexPath = MeshComp->GetMaterialPath().c_str();

					if (ImGui::InputText("Texture Path", &CurrentTexPath, ImGuiInputTextFlags_EnterReturnsTrue))
					{
						MeshComp->SetMaterial(FString(CurrentTexPath.c_str()));
					}

					ImGui::TextDisabled("Type texture path and press Enter.");
				}
				if (ImGui::Button("Delete"))
				{
					DeleteSelectedActor();
				}
			}
			ImGui::End();
		}
	}
	SetSelectedValue(bRotationChanged);
	bEditingRotation = SelectedComponent != nullptr && bRotationActive;
}
