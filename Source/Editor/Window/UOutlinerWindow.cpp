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
	SelectedComponent = Editor->GetSelectedSceneComponent();
	ImGui::Begin("Outliner");

	UScene* Scene = Editor->GetCurrentScene();
	if (Scene == nullptr)
	{
		ImGui::End();
		return;
	}

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
