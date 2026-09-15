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

			for (UActorComponent* Component : Actor->GetComponents())
			{
				const FString ComponentLabel = std::format(
					"{}##Component{}", Component->GetName().ToString(), Component->GetUUID());

				if (!Component->IsA(USceneComponent::GetClass()))
				{
					ImGui::TextDisabled("%s", ComponentLabel.c_str());
					continue;
				}

				USceneComponent* SceneComponent = static_cast<USceneComponent*>(Component);
				if (ImGui::Selectable(ComponentLabel.c_str(),
					Editor->GetSelectedSceneComponent() == SceneComponent))
				{
					Editor->SetSelectedSceneComponent(SceneComponent);
				}
			}
			ImGui::TreePop();
		});

	ImGui::End();
}
