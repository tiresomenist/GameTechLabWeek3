#pragma once

struct FVertexText
{
	float X, Y, Z; //월드공간 최종 위치
	float U, V; // 폰트 아틀라스 내 UV 좌표
	float R, G, B, A; //정점 색상(알파 포함)
};