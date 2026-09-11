#pragma once
#include "Engine/Core.h"
#include "Engine/Renderer/FVertexSimple.h"

inline const FVertexSimple grid_vertices[] = {
	{ -1152.463135f, -1152.463135f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f },
	{ 1152.463135f, -1152.463135f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f },
	{ -1152.463135f, 1152.463135f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f },
	{ 1152.463135f, 1152.463135f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f },
};

inline const uint32 grid_indices[] = {
	0, 1, 3,
	0, 3, 2,
};
