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
				Editor->SetSelectedActor(Actor);
			}
		});

	Scene->ForEachPrimitive([&](UPrimitiveComponent* Component) -> void
		{
			USceneComponent* SC = static_cast<USceneComponent*>(Component);
			const FString& Name = SC->GetName().ToString();
			bool bSelected = SelectedComponent == SC;
			if (ImGui::Selectable(Name.c_str(), bSelected))
			{
				SelectedComponent = SC;
				Editor->SetSelectedSceneComponent(SelectedComponent);
			}
		});
	ImGui::End();
}
