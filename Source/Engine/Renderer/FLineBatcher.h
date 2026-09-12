#pragma once
#include <d3d11.h>
#include "FVertexSimple.h"
#include "Core/Container/TArray.h"
#include "Core/Math/FVector.h"
#include "Core/Math/Matrix.h"

class FLineBatcher
{
public:
	FLineBatcher() = default;
	~FLineBatcher() = default;

	void Initialize(ID3D11Device* Device);
	
	void AddLine(const FVertexSimple& Start, const FVertexSimple& End);
	void AddBoundingBox(const FVector& Max, const FVector& Min, const FMatrix& WorldMatrix, const FVector4& Color = FVector4(1.0f, 1.0f, 0.0f, 1.0f));
	void AddGrid(FVector CamPos, float GridSize, float Step);
	void AddWorldGizmo(float GridSize, float Step);

	UINT UpdateBuffers(ID3D11DeviceContext* Context);

	ID3D11Buffer* GetVertexBuffer() const { return VertexBuffer; }
	ID3D11Buffer* GetIndexBuffer() const { return IndexBuffer; }

	void Flush(ID3D11DeviceContext* Context, const FMatrix& ViewProjMatrix);
		//addgrid
		//flush
	void Release();

private:
	static constexpr uint32 MaxVertices = 20000;
	static constexpr uint32 MaxIndices = 60000;

	TArray<FVertexSimple> Vertices;
	TArray<uint32> Indices;

	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;
};