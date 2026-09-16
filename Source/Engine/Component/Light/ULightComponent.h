#pragma once

#include "Engine/Component/USceneComponent.h"

// 광원 컴포넌트의 공통 기반
class ULightComponent : public USceneComponent
{
    UCLASS(ULightComponent, "Light", USceneComponent)
};