#pragma once

#include "Core/Container/TArray.h"
#include "FVertexSimple.h"

class FLineBatcher
{
public:
	void AddLine(const FVertexSimple& A, const FVertexSimple& B);
	bool Build();
	void Clear();
	void Release();
	ID3D11Buffer* GetVertexBuffer() const;
	UINT GetVertexCount() const;

private:
	TArray<FVertexSimple> Vertices;

	ID3D11Buffer* VertexBuffer = nullptr;
	size_t BufferCapacity = 0;

	bool ReserveBuffer(size_t Capacity);
};
