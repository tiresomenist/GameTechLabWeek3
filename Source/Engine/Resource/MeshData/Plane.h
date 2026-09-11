#pragma once
#include "Core/Core.h"
#include "Engine/Renderer/FVertexSimple.h"

inline const FVertexSimple plane_vertices[] = {
    { -1.000000f, -1.000000f, 0.000000f, 1.000000f, 1.000000f, 0.000000f, 0.000000f },
    { 1.000000f, -1.000000f, 0.000000f, 1.000000f, 0.000000f, 1.000000f, 0.000000f },
    { -1.000000f, 1.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 1.000000f },
    { 1.000000f, 1.000000f, 0.000000f, 1.000000f, 1.000000f, 0.000000f, 0.000000f },
};

inline const uint32 plane_indices[] = {
    0, 1, 3,
    0, 3, 2,
};