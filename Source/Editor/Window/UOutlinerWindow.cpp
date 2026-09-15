#include "pch.h"
#include "UOutlinerWindow.h"
#include "Editor/FEditor.h"
#include "Engine/Scene/UScene.h"
#include "Engine/Actor/AActor.h"
#include "Engine/Component/UActorComponent.h"
#include "Engine/Component/USceneComponent.h"

void UOutlinerWindow::Initialize(FEditor* InEditor)
{
	UEditorWindow::Initialize(InEditor);
}

void UOutlinerWindow::Render(float DeltaTime)
{
	AActor* SelectedActor = Editor->GetSelectedActor();
	ImGui::Begin("Outliner");

	UScene* Scene = Editor->GetCurrentScene();
	if (Scene == nullptr)
	{
		ImGui::End();
		return;
	}

	Scene->ForEachActor([&](AActor* Actor)
		{
			const FString ActorLabel = std::format("{}##Actor{}", Actor->GetName().ToString(), Actor->GetUUID());
			const ImGuiTreeNodeFlags ActorFlags =
				ImGuiTreeNodeFlags_OpenOnArrow |
				ImGuiTreeNodeFlags_OpenOnDoubleClick |
				ImGuiTreeNodeFlags_DefaultOpen |
				(SelectedActor == Actor ? ImGuiTreeNodeFlags_Selected : 0);

			const bool bOpen = ImGui::TreeNodeEx(ActorLabel.c_str(), ActorFlags);
			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			{
				// Actor Transform은 RootComponent가 대표한다. 빈 Actor에는 기즈모를 띄우지 않는다.
				if (USceneComponent* Root = Actor->GetRootComponent())
				{
					Editor->SetSelectedSceneComponent(Root);
				}
				else
				{
					Editor->SetSelectedActor(Actor);
				}
			}

			if (!bOpen) return;

			auto RenderComponentTree = [&](auto&& Self, USceneComponent* Component, bool bIsRoot) -> void
			{
				const FString DisplayName = bIsRoot
					? std::format("{} (Root)", Component->GetName().ToString())
					: Component->GetName().ToString();
				const FString ComponentLabel = std::format(
					"{}##Component{}", DisplayName, Component->GetUUID());
				const bool bHasChildren = !Component->GetAttachChildren().empty();
				const ImGuiTreeNodeFlags ComponentFlags = bHasChildren
					? ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen
					: ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;

				const bool bComponentOpen = ImGui::TreeNodeEx(
					ComponentLabel.c_str(),
					ComponentFlags | (Editor->GetSelectedSceneComponent() == Component ? ImGuiTreeNodeFlags_Selected : 0));
				if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
				{
					Editor->SetSelectedSceneComponent(Component);
				}

				if (bHasChildren && bComponentOpen)
				{
					for (USceneComponent* Child : Component->GetAttachChildren())
					{
						if (Child != nullptr) Self(Self, Child, false);
					}
					ImGui::TreePop();
				}
			};

			if (USceneComponent* Root = Actor->GetRootComponent())
			{
				RenderComponentTree(RenderComponentTree, Root, true);
			}

			// Root에 연결되지 않은 SceneComponent도 잃지 않고 Actor 바로 아래에 표시한다.
			for (UActorComponent* Component : Actor->GetComponents())
			{
				if (!Component->IsA(USceneComponent::GetClass())) continue;

				USceneComponent* SceneComponent = static_cast<USceneComponent*>(Component);
				if (SceneComponent == Actor->GetRootComponent() || SceneComponent->GetAttachParent() != nullptr)
				{
					continue;
				}

				// UUID Widget처럼 Root가 될 수 없는 보조 컴포넌트는 편집 대상이 아님을 표시한다.
				if (!SceneComponent->CanBeRootComponent())
				{
					const FString HelperLabel = std::format(
						"{} (Helper)##Component{}", SceneComponent->GetName().ToString(), SceneComponent->GetUUID());
					ImGui::TextDisabled("%s", HelperLabel.c_str());
					continue;
				}

				RenderComponentTree(RenderComponentTree, SceneComponent, false);
			}
			ImGui::TreePop();
		});

	ImGui::End();
}
