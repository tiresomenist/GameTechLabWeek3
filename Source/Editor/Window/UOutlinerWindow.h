#pragma once

#include "Editor/Window/UEditorWindow.h"

class USceneComponent;

class UOutlinerWindow : public UEditorWindow
{
	UCLASS(UOutlinerWindow, "OutlinerWindow", UEditorWindow)
public:
	virtual void Initialize(FEditor* InEditor) override;
	virtual void Render(float DeltaTime) override;

private:
	UScene* Scene = nullptr;
	USceneComponent* SelectedComponent;
};
