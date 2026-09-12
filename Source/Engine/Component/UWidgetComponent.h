#pragma once

#include "Engine/Component/USceneComponent.h"
#include "Engine/Renderer/Text/FWorldTextItem.h"

class UCameraComponent;

// Primitive Actor의 RootComponent UUID를 빌보드 라벨로 표시한다.
class UWidgetComponent : public USceneComponent
{
	UCLASS(UWidgetComponent, "WidgetComponent", USceneComponent)

public:
	virtual bool CanBeRootComponent() const override { return false; }
	bool BuildTextItem(const UCameraComponent* Camera, FWorldTextItem& OutItem) const;
};
