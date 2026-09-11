#pragma once

struct FVertexSimple
{
    float x, y, z;    // Position
    float r, g, b, a; // Color
};

struct FVertexTest
{
	float x, y, z;      // Position
	float nx, ny, nz;   // Normal (빛 테스트용)
	float u, v;         // UV (텍스처 테스트용)
};