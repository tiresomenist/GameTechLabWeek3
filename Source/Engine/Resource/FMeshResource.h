#pragma once

#include <d3d11.h>

#include "Core/Container/TArray.h"
#include "Core/Core.h"
#include "Core/Math/FVector.h"


struct FMeshResource
{
    friend class GResourceManager;
public:
    FMeshResource() = default;
    FMeshResource(const FMeshResource&) = delete;
    FMeshResource& operator=(const FMeshResource&) = delete;
    ~FMeshResource()
    {
        if (VertexBuffer) VertexBuffer->Release();
        if (IndexBuffer) IndexBuffer->Release();
    }
    ID3D11Buffer* GetVertexBuffer() const { return VertexBuffer; }
    ID3D11Buffer* GetIndexBuffer() const { return IndexBuffer; }
    UINT GetVertexCount() const { return VertexCount; }
    UINT GetIndexCount() const { return IndexCount; }
    UINT GetStride() const { return Stride; }
    const TArray<FVector>& GetPositions() const { return Positions; }
    const TArray<uint32>& GetIndices() const { return indexes; }
    const FVector& GetBoundsMin() const { return BoundsMin; }
    const FVector& GetBoundsMax() const { return BoundsMax; }
    bool HasBounds() const { return bHasBounds; }
private:
	ID3D11Buffer* VertexBuffer = nullptr;
	ID3D11Buffer* IndexBuffer = nullptr;
	UINT VertexCount = 0;
	UINT IndexCount = 0;
	UINT Stride = 0;

	TArray<FVector> Positions;
	TArray<uint32> indexes;

	FVector BoundsMin;
	FVector BoundsMax;
	bool bHasBounds = false;
};
