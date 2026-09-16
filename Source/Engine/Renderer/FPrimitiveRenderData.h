#pragma once

#include <d3d11.h>
#include "Core/Math/FVector.h"

struct FMatrix; 
enum class EPrimitivePipeline : uint32
{
	Color,
	Texture,
};

enum class EPrimitiveBlendMode : uint32
{
	Opaque,
	Additive,
};

// HLSL의 float2 크기와 float2 오프셋에 대응하는 16바이트 상수
struct FTextureUVTransform
{
    float ScaleU = 1.0f;
    float ScaleV = 1.0f;
    float OffsetU = 0.0f;
    float OffsetV = 0.0f;
};
static_assert(sizeof(FTextureUVTransform) == 16);

struct FTextureDrawConstants
{
	FTextureUVTransform UV;

	// 텍스처에 곱할 오브젝트별 색상
	FVector4 Tint{ 1.0f, 1.0f, 1.0f, 1.0f };

	// 해당 값보다 작은 알파의 픽셀을 제거함
	float AlphaCutoff = 0.0f;

	// 상수 버퍼 크기를 16바이트 배수로 맞춤
	float Padding[3]{};
};
static_assert(sizeof(FTextureDrawConstants) == 48);


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

	FVector Min;
	FVector Max;
	//어느 셰이더 파이프라인을 사용할것인가?
	EPrimitivePipeline Pipeline = EPrimitivePipeline::Color;
	//어떤 블렌딩 모드를 사용할것인가?
	EPrimitiveBlendMode BlendMode = EPrimitiveBlendMode::Opaque;
    // 기본값은 텍스처 전체를 사용함
    FTextureUVTransform UVTransform;

	// 텍스처에 곱할 색상
	FVector4 TextureTint{ 1.0f, 1.0f, 1.0f, 1.0f };

	// 0이면 알파 컷아웃을 사용하지 않음
	float AlphaCutoff = 0.0f;

	// 선택 시 일반 메시 외곽선 렌더링 허용 여부
	bool bAllowOutline = true;

	// 구형 닫힌 메시에는 cull_back, 플립북, 평면, 빌보드에는 cull_none
	bool bTwoSided = false;
};
