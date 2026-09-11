#pragma once

#include <d3d11.h>

enum class ERenderPass : UINT8
{
	Opaque,		// 불투명
	Translucent,// 텍스트, 반투명 메시
};

struct FMatrix;

struct FPrimitiveRenderData
{
	ID3D11Buffer*				VertexBuffer = nullptr;
	ID3D11Buffer*				IndexBuffer = nullptr;
	UINT						Stride = 0;
	UINT						IndexCount = 0;
	D3D11_PRIMITIVE_TOPOLOGY	Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	ID3D11ShaderResourceView*	Material = nullptr;			// VS/PS, 텍스처 SRV 등을 들고 있는 객체
	const FMatrix*				WorldMatrix = nullptr;		// 컴포넌트가 소유한 월드행렬 가리키기

	bool						isSelected = false;

	ERenderPass RenderPass =	ERenderPass::Opaque;
};