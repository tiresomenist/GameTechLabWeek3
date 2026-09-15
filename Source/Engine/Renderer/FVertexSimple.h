#pragma once

struct FVertexSimple
{
	float x = 0.0f, y = 0.0f, z = 0.0f;    // Position
	float r = 0.0f, g = 0.0f, b = 0.0f, a = 1.0f; // Color
};

struct FVertexTest
{
	float x, y, z;      // Position
	float nx, ny, nz;   // Normal (빛 테스트용)
	float u, v;         // UV (텍스처 테스트용)
};

struct FVertexTexture {
	float x = 0.0f, y = 0.0f, z = 0.0f;
	float r = 0.0f, g = 0.0f, b = 0.0f, a = 1.0f; // Color
	float u = 0.0f, v = 0.0f;
};

struct FVertexText
{
	float X, Y, Z; //월드공간 최종 위치
	float U, V; // 폰트 아틀라스 내 UV 좌표
	float R, G, B, A; //정점 색상(알파 포함)
};