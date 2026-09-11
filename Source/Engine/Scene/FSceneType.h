#pragma once

#include "Core/Container/FString.h"

class UScene;

// UObject RTTI와 분리된 Scene 생성 정보입니다.
struct FSceneType
{
    FString Name;
    UScene* (*SceneConstructor)() = nullptr;
};
