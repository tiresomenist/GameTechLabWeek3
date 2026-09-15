#include "pch.h"
#include "UOutlinerWindow.h"
#include "Editor/FEditor.h"
#include "Engine/Scene/UScene.h"

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
			const FString Label = std::format("{}##{}", Actor->GetName().ToString(), Actor->GetUUID());

			if (ImGui::Selectable(Label.c_str(), SelectedActor == Actor))
			{
				if (USceneComponent* Root = Actor->GetRootComponent())
				{
					Editor->SetSelectedSceneComponent(Root);
				}
				else
				{
					// 빈 Actor: 선택은 되지만 움직일 Transform이 없으므로 기즈모 없음
					Editor->SetSelectedActor(Actor);
				}
			}
		});

	ImGui::End();
}
