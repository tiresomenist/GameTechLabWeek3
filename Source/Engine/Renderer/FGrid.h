#pragma once

#include "Core/Core.h"

struct FGrid
{
	float Interval = 1.0f;
	// Half of the grid's world-space width; independent of Interval.
	float Extent = 50.0f;
	static constexpr float MinInterval = 0.1f;
	static constexpr float MaxInterval = 60.0f;
};
