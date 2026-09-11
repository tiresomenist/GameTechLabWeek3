#pragma once

#include "UEditorWindow.h"
#include "Core/Container/FString.h"
#include "Core/Container/TDeque.h"
#include "Engine/FConsole.h"
#include "Core/Core.h"
#include "ImGui/imgui.h"
#include "Core/Math/Matrix.h"

class UConsoleWindow : public UEditorWindow
{
    UCLASS(UConsoleWindow, "ConsoleWindow", UEditorWindow)

private:
	ImGuiTextFilter Filter;
	TDeque<FString> logs;
public:
	void AddDebugText(FString DebugText);
	void AddDebugError(FString ErrorText);
	void Clear();
	void Copy();
	void Option();

	void Render(float DeltaTime) override;
};

