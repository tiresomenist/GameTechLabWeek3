#pragma once
#include "Engine/Core.h"
#include "Engine/Renderer/FVertexSimple.h"

// 삼각형을 하드 코딩

inline const FVertexSimple triangle_vertices[] = {
	{ -0.665393f, -0.686524f, 0.182501f, 0.665393f, 1.0f, 0.0f, 0.0f },
	{ 1.334607f, -0.686524f, 0.182501f, 1.000000f, 0.0f, 1.0f, 0.0f },
	{ -0.665393f, 1.313476f, 0.182501f, 0.665393f, 0.0f, 0.0f, 1.0f },
};

inline const uint32 triangle_indices[] = {
	1, 2, 0,
};