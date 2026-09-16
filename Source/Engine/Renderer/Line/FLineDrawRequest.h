#pragma once

#include "Core/Container/TArray.h"
#include "Engine/Renderer/FVertexSimple.h"
#include <functional>

class UCameraComponent;

// 하나의 오브젝트가 생성한 라인 메시 요청
struct FLineDrawRequest
{
    // 월드 좌표의 정점과 색상을 보관함
    TArray<FVertexSimple> Vertices;

    // 요청 내부 정점 배열을 기준으로 선의 연결 관계를 보관함
    TArray<uint32> Indices;
};

using FLineRequestConsumer = std::function<void(const FLineDrawRequest&)>;

// 오브젝트가 라인 요청을 생성할 때 필요한 표시 정보
struct FLineDrawContext
{
    const UCameraComponent* Camera = nullptr;

    bool bSelected = false;
    bool bShowBounds = false;
    bool bShowPrimitives = true;
};