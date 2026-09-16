#pragma once
#include <d3d11.h>
#include "Core/Container/TArray.h"
#include "Engine/Renderer/Line/FLineDrawRequest.h"

class FLineBatcher
{
public:
	bool AddRequest(const FLineDrawRequest& Request);
	bool Build();
	void Clear();
	void Release();

	ID3D11Buffer* GetVertexBuffer() const;
	ID3D11Buffer* GetIndexBuffer() const;

	UINT GetVertexCount() const;
	UINT GetIndexCount() const;

private:
	TArray<FVertexSimple> Vertices;
	TArray<uint32> Indices;

	ID3D11Buffer* VertexBuffer = nullptr;
	ID3D11Buffer* IndexBuffer = nullptr;

	size_t VertexBufferCapacity = 0;
	size_t IndexBufferCapacity = 0;

	bool ReserveVertexBuffer(size_t Need);
	bool ReserveIndexBuffer(size_t Need);
};
