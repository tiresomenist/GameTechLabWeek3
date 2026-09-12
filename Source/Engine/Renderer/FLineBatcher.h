#pragma once

#include "Core/Container/TArray.h"
#include "Core/Math/FVector.h"
#include "Core/Math/Matrix.h"
#include "FVertexSimple.h"

class FLineBatcher
{
public:
	void AddLine(const FVertexSimple& A, const FVertexSimple& B);
	void AddBoundBox(const FVector& Min, const FVector& Max, const FMatrix& World);
	bool Build();
	void Clear();
	void Release();
	ID3D11Buffer* GetVertexBuffer() const;
	ID3D11Buffer* GetIndexBuffer() const;
	UINT GetVertexCount() const;
	UINT GetIndexCount() const;

private:
	TArray<FVertexSimple> Vertices;
	TArray<uint32> Indexes;

	ID3D11Buffer* VertexBuffer = nullptr;
	ID3D11Buffer* IndexBuffer = nullptr;

	size_t VertexBufferCapacity = 0;
	size_t IndexBufferCapacity = 0;

	bool ReserveVertexBuffer(size_t Need);
	bool ReserveIndexBuffer(size_t Need);
};
