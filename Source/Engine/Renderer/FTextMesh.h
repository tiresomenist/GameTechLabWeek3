#pragma once

#include "Core/Math/FVector.h"
#include "Core/Container/FString.h"
#include "Engine/Resource/FFontAtlas.h"
#include "Engine/Renderer/FVertexText.h"

enum class ETextAlign { Left, Center, Right };

// 세로 기준(피벗): 텍스트 박스의 어디를 원점에 둘지
enum class ETextVAlign { Top, Center, Bottom };

struct FTextStyle
{
	float Size = 1.0f;                         // 월드 단위. 구운 높이(32px)가 이 크기가 됨
	FVector4 Color = FVector4(1, 1, 1, 1);
	ETextAlign Align = ETextAlign::Left;
	ETextVAlign VAlign = ETextVAlign::Top;
};

class FTextMesh
{
public:
	~FTextMesh() { Release(); }
	FTextMesh() = default;
	FTextMesh(const FTextMesh&) = delete;              // COM 포인터를 소유하니까 (FTexture와 같은 이유)
	FTextMesh& operator=(const FTextMesh&) = delete;

	// 문자열 → Vertices / Indices 채우기. GPU와 무관
	void Build(const FFontAtlas& Font, const FString& Text, const FTextStyle& Style);

	bool Upload(ID3D11Device* Device, ID3D11DeviceContext* Context);
	void Release();

	ID3D11Buffer* GetVertexBuffer() const { return VB; }
	ID3D11Buffer* GetIndexBuffer() const { return IB; }
	UINT GetIndexCount() const { return IndexCount; }

	const TArray<FVertexText>& GetVertices() const { return Vertices; }
	const TArray<uint32>& GetIndices() const { return Indices; }

	// 로컬 공간 AABB (피킹용). 그릴 쿼드가 없으면 HasBounds() == false
	bool HasBounds() const { return bHasBounds; }
	const FVector& GetBoundsMin() const { return BoundsMin; }
	const FVector& GetBoundsMax() const { return BoundsMax; }

private:
	TArray<FVertexText> Vertices;
	TArray<uint32> Indices;

	ID3D11Buffer* VB = nullptr;
	ID3D11Buffer* IB = nullptr;
	UINT VBCapacity = 0;    // 버퍼에 들어갈 수 있는 정점 수
	UINT IBCapacity = 0;    // 인덱스 수
	UINT IndexCount = 0;    // 실제로 그릴 인덱스 수

	FVector BoundsMin;
	FVector BoundsMax;
	bool bHasBounds = false;
};