#pragma once
#include "Core/Core.h"
#include "Engine/Renderer/FVertexTexture.h"

inline const FVertexTexture plane_vertices[] = {
    { -1.000000f, -1.000000f, 0.000000f, 0.0f, 1.0f },
    { 1.000000f, -1.000000f, 0.000000f, 1.0f, 1.0f },
    { -1.000000f, 1.000000f, 0.000000f, 0.0f, 0.0f },
    { 1.000000f, 1.000000f, 0.000000f, 1.0f, 0.0f },
};

inline const uint32 plane_indices[] = {
    0, 1, 3,
    0, 3, 2,
};
