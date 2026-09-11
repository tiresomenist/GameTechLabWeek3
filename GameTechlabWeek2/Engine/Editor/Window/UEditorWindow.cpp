#include "pch.h"
#include "UEditorWindow.h"

void UEditorWindow::Initialize(FEditor* InEditor)
{
	Editor = InEditor;
}

void UEditorWindow::DrawItemBottomLine(uint32 Color, float Thickness)
{
	ImVec2 Min = ImGui::GetItemRectMin();
	ImVec2 Max = ImGui::GetItemRectMax();

	ImGui::GetWindowDrawList()->AddLine(
		ImVec2(Min.x, Max.y),
		ImVec2(Max.x, Max.y),
		Color,
		Thickness
	);
}
