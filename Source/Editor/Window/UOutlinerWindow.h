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
	// 씬은 로드 시 교체되므로 포인터를 캐시하지 않고 매 프레임 Editor에서 가져온다
	USceneComponent* SelectedComponent = nullptr;
};
