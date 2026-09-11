#pragma once

// Font quads use POSITION(float3) + TEXCOORD(float2).
struct FFontVertex
{
    float X, Y, Z;
    float U, V;
};
static_assert(sizeof(FFontVertex) == 20);

struct FVertexSimple
{
    float x, y, z;    // Position
    float r, g, b, a; // Color
};
