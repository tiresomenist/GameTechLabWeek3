#pragma once

#include "Core/Container/TArray.h"
#include "Engine/Renderer/FVertexSimple.h"

// XY 평면의 사각형에 텍스처 전체를 매핑하는 정점 데이터
inline const TArray<FVertexTexture> flame_vertices =
{
    { -1.f, -1.f, 0.f,  1.f, 1.f, 1.f, 1.f,  0.f, 1.f },
    { 1.f, -1.f, 0.f,   1.f, 1.f, 1.f, 1.f,  1.f, 1.f },
    { -1.f,  1.f, 0.f,  1.f, 1.f, 1.f, 1.f,  0.f, 0.f },
    { 1.f,  1.f, 0.f,   1.f, 1.f, 1.f, 1.f,  1.f, 0.f },
};

// 두 삼각형이 네 정점을 공유함
inline const TArray<uint32> flame_indices =
{
    0, 1, 3,
    0, 3, 2,
};
