#pragma once

#include <d3d11.h>

struct FMatrix;

struct FPrimitiveRenderData
{
	ID3D11Buffer*				VertexBuffer = nullptr;
	ID3D11Buffer*				IndexBuffer = nullptr;
	UINT						Stride = 0;
	UINT						IndexCount = 0;
	UINT						StartIndexLocation = 0;		// 텍스트에서 공유되는 인덱스 버퍼에서의 시작점
	D3D11_PRIMITIVE_TOPOLOGY	Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	ID3D11ShaderResourceView*	Material = nullptr;			// 리소스 소유자가 제공하는 텍스처 SRV (비소유 참조)
	const FMatrix*				WorldMatrix = nullptr;		// 컴포넌트가 소유한 월드행렬 가리키기

	bool						isSelected = false;
};
