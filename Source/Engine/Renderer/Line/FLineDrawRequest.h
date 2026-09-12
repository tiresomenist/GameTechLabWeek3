#pragma once
#include "Core/Math/FVector.h"
#include "Core/Container/TArray.h"


struct FLineDrawRequest
{
    // World-space points; each pair of local indices describes one line.
    TArray<FVector> Points;
    TArray<uint32> Indices;

    float R = 1.0f;
    float G = 1.0f;
    float B = 1.0f;
    float A = 1.0f;
};
